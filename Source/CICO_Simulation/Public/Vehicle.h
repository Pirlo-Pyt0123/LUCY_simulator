// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Vehicle.generated.h"

UCLASS()
class CICO_SIMULATION_API AVehicle : public AActor
{
	GENERATED_BODY()

public:
	AVehicle();

	// Caja de colision raiz (confiable, no depende del Physics Asset)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	UBoxComponent* CollisionBox;

	// Mesh del vehiculo (cuelga de CollisionBox)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	USkeletalMeshComponent* VehicleMesh;

	// Tamanio de la caja de colision (ajustable desde el editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector BoxExtent;

	// Variantes de vehiculo — se elige uno al azar en BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Visual")
	TArray<USkeletalMesh*> VehicleMeshVariants;

	// Waypoints de ruta (loop automatico)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	TArray<AActor*> Waypoints;

	// Velocidad de desplazamiento en cm/s
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	float MoveSpeed;

	// Distancia para considerar que llego al waypoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	float WaypointAcceptRadius;

	// Segundos que espera en cada waypoint (simula semaforo o stop)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	float WaitTime;

	// Velocidad de interpolacion de rotacion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	float RotationSpeed;

	// Distancia en cm a la que el vehiculo frena para no atropellar al jugador
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Route")
	float StopForPlayerDistance;

	// Sonido cercano (se activa cuando el jugador esta muy cerca)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Audio")
	USoundBase* ProximitySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Audio")
	float ProximityDistance;

	// Sonido lejano (se activa cuando el jugador esta a mayor distancia)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Audio")
	USoundBase* ProximitySound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Audio")
	float ProximityDistance2;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	int32 CurrentWaypointIndex;
	bool  bIsWaiting;
	bool  bProximityActive;
	bool  bProximityActive2;
	bool  bStoppedForPlayer;
	FTimerHandle WaitTimerHandle;

	UAudioComponent* ProximitySoundComponent;
	UAudioComponent* ProximitySoundComponent2;

	void ApplyRandomMesh();
	void OnWaitFinished();
	void UpdateProximitySound();
};
