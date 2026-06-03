// Fill out your copyright notice in the Description page of Project Settings.

#include "Pedestrian.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APedestrian::APedestrian()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsula estandar de personaje humano
	GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
	GetCapsuleComponent()->SetCapsuleRadius(34.f);

	// Posicion y rotacion del mesh para que quede alineado con la capsula
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Defaults de patrulla
	WaypointAcceptRadius = 100.f;
	WaitTime             = 2.f;
	bRandomBehavior      = true;
	Behavior             = EPedestrianBehavior::Walk;
	CurrentWaypointIndex = 0;
	bIsWaiting           = false;
}

void APedestrian::BeginPlay()
{
	Super::BeginPlay();

	ApplyRandomMesh();
	ApplyBehavior();
}

void APedestrian::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsWaiting || Behavior == EPedestrianBehavior::Idle || Waypoints.Num() == 0)
		return;

	AActor* Target = Waypoints[CurrentWaypointIndex];
	if (!Target) return;

	FVector CurrentLoc = GetActorLocation();
	FVector TargetLoc  = Target->GetActorLocation();
	float   Dist       = FVector::Dist2D(CurrentLoc, TargetLoc);

	// Llego al waypoint — esperar y pasar al siguiente
	if (Dist < WaypointAcceptRadius)
	{
		bIsWaiting = true;
		GetCharacterMovement()->StopMovementImmediately();
		GetWorldTimerManager().SetTimer(
			WaitTimerHandle, this, &APedestrian::OnWaitFinished, WaitTime, false);
		return;
	}

	// Direccion hacia el waypoint (ignorar Z para no inclinarse)
	FVector Direction = (TargetLoc - CurrentLoc);
	Direction.Z = 0.f;
	Direction.Normalize();

	// AddMovementInput alimenta el CharacterMovement → ABP_Manny detecta la velocidad
	AddMovementInput(Direction, 1.f);

	// Rotar suavemente hacia donde camina
	FRotator TargetRot = Direction.Rotation();
	FRotator NewRot    = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 8.f);
	SetActorRotation(NewRot);
}

void APedestrian::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APedestrian::ApplyRandomMesh()
{
	if (MeshVariants.Num() == 0) return;

	int32 RandIndex          = FMath::RandRange(0, MeshVariants.Num() - 1);
	USkeletalMesh* ChosenMesh = MeshVariants[RandIndex];

	if (ChosenMesh)
	{
		GetMesh()->SetSkeletalMesh(ChosenMesh);
	}

	if (AnimClass)
	{
		GetMesh()->SetAnimInstanceClass(AnimClass);
	}
}

void APedestrian::ApplyBehavior()
{
	if (bRandomBehavior)
	{
		// 20% idle | 50% walk | 30% run
		float Rand = FMath::FRand();
		if      (Rand < 0.20f) Behavior = EPedestrianBehavior::Idle;
		else if (Rand < 0.70f) Behavior = EPedestrianBehavior::Walk;
		else                   Behavior = EPedestrianBehavior::Run;
	}

	// MaxWalkSpeed controla que animacion activa ABP_Manny segun la velocidad real
	switch (Behavior)
	{
		case EPedestrianBehavior::Idle:
			GetCharacterMovement()->MaxWalkSpeed = 0.f;
			break;
		case EPedestrianBehavior::Walk:
			GetCharacterMovement()->MaxWalkSpeed = 150.f;
			break;
		case EPedestrianBehavior::Run:
			GetCharacterMovement()->MaxWalkSpeed = 500.f;
			break;
	}
}

void APedestrian::OnWaitFinished()
{
	bIsWaiting           = false;
	CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
}
