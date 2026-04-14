// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LUCY.generated.h"

class ACharacter;

UCLASS()
class CICO_SIMULATION_API ALUCY : public AActor
{
	GENERATED_BODY()

public:
	ALUCY();

	// Mesh del dron
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lucy")
	UStaticMeshComponent* MeshLUCY;

	// Camara de vigilancia (captura frames para el backend)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lucy|Camera")
	USceneCaptureComponent2D* CaptureCamera;

	// RenderTarget donde se vuelcan los frames (600x600 = resolucion del backend)
	UPROPERTY(BlueprintReadOnly, Category = "Lucy|Camera")
	UTextureRenderTarget2D* RenderTarget;

	// --- Seguimiento del jugador ---

	// Offset relativo al jugador (X=adelante, Y=derecha, Z=arriba)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Follow")
	FVector FollowOffset;

	// Velocidad de interpolacion del seguimiento
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Follow")
	float FollowSpeed;

	// Amplitud del bobbing vertical (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Follow")
	float BobAmplitude;

	// Frecuencia del bobbing (ciclos por segundo)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Follow")
	float BobFrequency;

	// --- Backend StealthVision ---

	// URL base del servidor (ej: http://localhost:8000)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Backend")
	FString ServerURL;

	// ID de sesion para el tracking (debe ser unico por camara)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Backend")
	FString SessionId;

	// Intervalo en segundos entre cada envio de frame al backend
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lucy|Backend")
	float CaptureInterval;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	ACharacter* PlayerCharacter;
	float ElapsedTime;
	FTimerHandle CaptureTimerHandle;

	// Captura el frame actual y lo envia al backend
	void CaptureAndSend();
};
