// Fill out your copyright notice in the Description page of Project Settings.

#include "LUCY.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
// HTTP
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
// JSON
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
// Imagen
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

ALUCY::ALUCY()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Mesh ---
	MeshLUCY = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshLUCY"));
	RootComponent = MeshLUCY;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshLUCYAsset(
		TEXT("/Script/Engine.StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshLUCYAsset.Succeeded())
	{
		MeshLUCY->SetStaticMesh(MeshLUCYAsset.Object);
	}

	// --- Camara de captura ---
	CaptureCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureCamera"));
	CaptureCamera->SetupAttachment(MeshLUCY);
	CaptureCamera->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
	CaptureCamera->bCaptureEveryFrame = false;
	CaptureCamera->bCaptureOnMovement = false;

	// --- Defaults de seguimiento ---
	FollowOffset   = FVector(30.f, 70.f, 90.f);
	FollowSpeed    = 5.f;
	BobAmplitude   = 5.f;
	BobFrequency   = 1.5f;

	// --- Defaults de backend ---
	ServerURL       = TEXT("http://localhost:8000");
	SessionId       = TEXT("lucy_cam");
	CaptureInterval = 1.0f;

	// --- Estado interno ---
	ElapsedTime     = 0.f;
	PlayerCharacter = nullptr;
	RenderTarget    = nullptr;
}

void ALUCY::BeginPlay()
{
	Super::BeginPlay();

	// Crear RenderTarget 600x600 (resolucion probada con el backend)
	RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("LucyRenderTarget"));
	RenderTarget->InitAutoFormat(600, 600);
	RenderTarget->UpdateResourceImmediate(true);

	if (CaptureCamera)
	{
		CaptureCamera->TextureTarget = RenderTarget;
	}

	// Buscar al jugador
	PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Arrancar el timer de envio al backend
	GetWorldTimerManager().SetTimer(
		CaptureTimerHandle,
		this,
		&ALUCY::CaptureAndSend,
		CaptureInterval,
		true  // loop
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Cyan,
			TEXT("[LUCY] Online — conectando con StealthVision..."));
	}
}

void ALUCY::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerCharacter) return;

	ElapsedTime += DeltaTime;

	// Posicion objetivo relativa al jugador (se adapta a su rotacion)
	FVector TargetLocation =
		PlayerCharacter->GetActorLocation()
		+ PlayerCharacter->GetActorForwardVector() * FollowOffset.X
		+ PlayerCharacter->GetActorRightVector()   * FollowOffset.Y
		+ FVector::UpVector                        * FollowOffset.Z;

	// Bobbing senoidal en Z
	TargetLocation.Z += FMath::Sin(ElapsedTime * BobFrequency * PI * 2.f) * BobAmplitude;

	// Interpolacion suave
	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, FollowSpeed);
	SetActorLocation(NewLocation);
}

void ALUCY::CaptureAndSend()
{
	if (!CaptureCamera || !RenderTarget) return;

	// 1. Capturar el frame actual
	CaptureCamera->CaptureScene();

	// 2. Leer los pixeles del RenderTarget
	FRenderTarget* RT = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RT) return;

	TArray<FColor> Pixels;
	if (!RT->ReadPixels(Pixels)) return;

	// 3. Encodear como JPEG
	IImageWrapperModule& ImageWrapperModule =
		FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	TSharedPtr<IImageWrapper> ImageWrapper =
		ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!ImageWrapper.IsValid()) return;

	ImageWrapper->SetRaw(
		Pixels.GetData(),
		Pixels.Num() * sizeof(FColor),
		600, 600,
		ERGBFormat::BGRA,
		8
	);

	TArray64<uint8> JpegData64 = ImageWrapper->GetCompressed(85);
	if (JpegData64.Num() == 0) return;

	// Copiar a TArray<uint8> para el body HTTP
	TArray<uint8> JpegBytes(JpegData64.GetData(), JpegData64.Num());

	// 4. Construir el body multipart/form-data
	FString Boundary = TEXT("LucyCaptureBoundary");

	TArray<uint8> Body;

	FString PartHeader = FString::Printf(
		TEXT("--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"frame.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n"),
		*Boundary);
	FTCHARToUTF8 PartHeaderUtf8(*PartHeader);
	Body.Append((uint8*)PartHeaderUtf8.Get(), PartHeaderUtf8.Length());

	Body.Append(JpegBytes);

	FString Closing = FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary);
	FTCHARToUTF8 ClosingUtf8(*Closing);
	Body.Append((uint8*)ClosingUtf8.Get(), ClosingUtf8.Length());

	// 5. Armar y enviar la request HTTP
	double UnixTimestamp = (double)FDateTime::UtcNow().ToUnixTimestamp();

	FString URL = FString::Printf(
		TEXT("%s/security?session_id=%s&timestamp=%f"),
		*ServerURL, *SessionId, UnixTimestamp);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest =
		FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(
		TEXT("Content-Type"),
		FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
	HttpRequest->SetContent(Body);

	// 6. Callback con la respuesta — captura WeakThis para evitar crash si Lucy se destruye
	TWeakObjectPtr<ALUCY> WeakThis(this);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!WeakThis.IsValid()) return;

			if (!bSuccess || !Response.IsValid())
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
						TEXT("[LUCY] Sin respuesta del backend"));
				}
				return;
			}

			// Parsear JSON
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Response->GetContentAsString());

			if (!FJsonSerializer::Deserialize(Reader, JsonObject)) return;

			FString RiskLevel = JsonObject->GetStringField(TEXT("risk_level"));
			FString Titulo    = JsonObject->GetStringField(TEXT("titulo"));

			// Color segun nivel de riesgo
			FColor DisplayColor = FColor::Green;
			if      (RiskLevel == TEXT("WARNING"))  DisplayColor = FColor::Yellow;
			else if (RiskLevel == TEXT("CRITICAL")) DisplayColor = FColor::Red;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					1,    // key fijo para que sobreescriba el mensaje anterior
					3.f,
					DisplayColor,
					FString::Printf(TEXT("[LUCY] %s — %s"), *RiskLevel, *Titulo));
			}
		}
	);

	HttpRequest->ProcessRequest();
}
