// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	int32 CurrentWaypointIndex;
	bool  bIsWaiting;
	FTimerHandle WaitTimerHandle;

	void ApplyRandomMesh();
	void OnWaitFinished();
};
