// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Pedestrian.generated.h"

UENUM(BlueprintType)
enum class EPedestrianBehavior : uint8
{
	Idle  UMETA(DisplayName = "Idle"),
	Walk  UMETA(DisplayName = "Walk"),
	Run   UMETA(DisplayName = "Run")
};

UCLASS()
class CICO_SIMULATION_API APedestrian : public ACharacter
{
	GENERATED_BODY()

public:
	APedestrian();

	// Meshes disponibles — se elige uno al azar en BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Visual")
	TArray<USkeletalMesh*> MeshVariants;

	// Animation Blueprint del pack CR (ABP_Manny)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Visual")
	TSubclassOf<UAnimInstance> AnimClass;

	// Waypoints de patrulla — dejar vacio para peatones estaticos
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Patrol")
	TArray<AActor*> Waypoints;

	// Distancia para considerar que llego al waypoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Patrol")
	float WaypointAcceptRadius;

	// Segundos que espera en cada waypoint antes de seguir
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Patrol")
	float WaitTime;

	// Si true el comportamiento se sortea al azar en BeginPlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Behavior")
	bool bRandomBehavior;

	// Comportamiento activo (se puede forzar desde el editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestrian|Behavior")
	EPedestrianBehavior Behavior;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	int32 CurrentWaypointIndex;
	bool  bIsWaiting;
	FTimerHandle WaitTimerHandle;

	void ApplyRandomMesh();
	void ApplyBehavior();
	void MoveToNextWaypoint();
	void OnWaitFinished();
};
