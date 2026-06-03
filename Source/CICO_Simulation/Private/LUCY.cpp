// Fill out your copyright notice in the Description page of Project Settings.

#include "LUCY.h"
#include "RiskHUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
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

	// --- Raiz invisible (SetActorRotation rota esta, no el mesh) ---
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// --- Mesh (cuelga de SceneRoot, conserva su propio offset de rotacion) ---
	MeshLUCY = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshLUCY"));
	MeshLUCY->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshLUCYAsset(
		TEXT("/Script/Engine.StaticMesh'/Game/Drone/drone.drone'"));
	if (MeshLUCYAsset.Succeeded())
	{
		MeshLUCY->SetStaticMesh(MeshLUCYAsset.Object);
		MeshLUCY->SetRelativeScale3D(FVector(0.20f, 0.20f, 0.20f));
	}

	// --- Camara de captura ---
	CaptureCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureCamera"));
	CaptureCamera->SetupAttachment(SceneRoot);
	CaptureCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	CaptureCamera->bCaptureEveryFrame = false;
	CaptureCamera->bCaptureOnMovement = false;
	// Imagen final post-procesada con tonemapping y gamma
	CaptureCamera->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// Forzar exposicion manual para que no calcule una exposicion distinta a la camara principal
	CaptureCamera->PostProcessBlendWeight = 1.0f;
	CaptureCamera->PostProcessSettings.bOverride_AutoExposureMethod = true;
	CaptureCamera->PostProcessSettings.AutoExposureMethod = AEM_Manual;
	CaptureCamera->PostProcessSettings.bOverride_AutoExposureBias = true;
	CaptureCamera->PostProcessSettings.AutoExposureBias = 0.f; // Subir si sigue oscuro (ej: 1.0, 2.0)

	// Rotacion del mesh (ajustable desde el editor)
	MeshRotationOffset = FRotator(0.f, -90.f, 0.f);

	// --- Defaults de seguimiento ---
	FollowOffset   = FVector(0.f, 90.f, 70.f); // Costado derecho mas alejado
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
	RiskHUDInstance = nullptr;
}

void ALUCY::BeginPlay()
{
	Super::BeginPlay();

	// Crear RenderTarget 600x600 en formato RGBA8 (gamma-correcto, compatible con JPEG)
	RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("LucyRenderTarget"));
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	RenderTarget->InitAutoFormat(600, 600);
	RenderTarget->UpdateResourceImmediate(true);

	if (CaptureCamera)
	{
		CaptureCamera->TextureTarget = RenderTarget;
	}

	// Aplicar rotacion del mesh desde la propiedad editable
	MeshLUCY->SetRelativeRotation(MeshRotationOffset);

	// Buscar al jugador
	PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Crear y mostrar el HUD de Lucy
	if (RiskHUDClass)
	{
		RiskHUDInstance = CreateWidget<URiskHUD>(GetWorld(), RiskHUDClass);
		if (RiskHUDInstance)
		{
			RiskHUDInstance->AddToViewport();
		}
	}

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

	// Interpolacion suave de posicion
	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, FollowSpeed);
	SetActorLocation(NewLocation);

	// Rotacion: Lucy mira hacia donde apunta la camara del jugador (ControlRotation)
	AController* Controller = PlayerCharacter->GetController();
	if (Controller)
	{
		FRotator TargetRotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, FollowSpeed);
		SetActorRotation(NewRotation);
	}
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

	// 6. Callback con la respuesta
	TWeakObjectPtr<ALUCY> WeakThis(this);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!WeakThis.IsValid()) return;

			if (!bSuccess || !Response.IsValid())
			{
				if (WeakThis->RiskHUDInstance)
				{
					WeakThis->RiskHUDInstance->UpdateRisk(
						TEXT("SAFE"),
						TEXT("Sin respuesta del backend"),
						TEXT("Verificar que el servidor este activo"));
				}
				return;
			}

			// Parsear JSON
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Response->GetContentAsString());

			if (!FJsonSerializer::Deserialize(Reader, JsonObject)) return;

			FString RiskLevel   = JsonObject->GetStringField(TEXT("risk_level"));
			FString Titulo      = JsonObject->GetStringField(TEXT("titulo"));
			FString Explicacion = JsonObject->GetStringField(TEXT("explicacion"));

			// Actualizar el HUD con los datos del backend
			if (WeakThis->RiskHUDInstance)
			{
				WeakThis->RiskHUDInstance->UpdateRisk(RiskLevel, Titulo, Explicacion);
			}
		}
	);

	HttpRequest->ProcessRequest();
}
