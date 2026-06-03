// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

AVehicle::AVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Caja de colision como raiz — confiable sin Physics Asset
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));

	// Tamanio por defecto para un sedan (ajustable en editor)
	BoxExtent = FVector(250.f, 110.f, 80.f);
	CollisionBox->SetBoxExtent(BoxExtent);

	// Mesh cuelga de la caja de colision
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetupAttachment(CollisionBox);
	VehicleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MoveSpeed            = 600.f;
	WaypointAcceptRadius = 200.f;
	WaitTime             = 0.f;
	RotationSpeed        = 4.f;
	CurrentWaypointIndex = 0;
	bIsWaiting           = false;
}

void AVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Aplicar el tamanio de caja configurado en el editor
	CollisionBox->SetBoxExtent(BoxExtent);

	ApplyRandomMesh();
}

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsWaiting || Waypoints.Num() == 0) return;

	AActor* Target = Waypoints[CurrentWaypointIndex];
	if (!Target) return;

	FVector CurrentLoc = GetActorLocation();
	FVector TargetLoc  = Target->GetActorLocation();
	TargetLoc.Z        = CurrentLoc.Z;

	float Distance = FVector::Dist2D(CurrentLoc, TargetLoc);

	if (Distance < WaypointAcceptRadius)
	{
		if (WaitTime > 0.f)
		{
			bIsWaiting = true;
			GetWorldTimerManager().SetTimer(
				WaitTimerHandle, this, &AVehicle::OnWaitFinished, WaitTime, false);
		}
		else
		{
			CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
		}
		return;
	}

	FVector Direction = (TargetLoc - CurrentLoc).GetSafeNormal();
	SetActorLocation(CurrentLoc + Direction * MoveSpeed * DeltaTime);

	FRotator TargetRotation = Direction.Rotation();
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);
}

void AVehicle::ApplyRandomMesh()
{
	if (VehicleMeshVariants.Num() == 0) return;

	int32 RandIndex       = FMath::RandRange(0, VehicleMeshVariants.Num() - 1);
	USkeletalMesh* ChosenMesh = VehicleMeshVariants[RandIndex];

	if (ChosenMesh && VehicleMesh)
	{
		VehicleMesh->SetSkeletalMesh(ChosenMesh);
	}
}

void AVehicle::OnWaitFinished()
{
	bIsWaiting           = false;
	CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
}
