// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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
	ProximityDistance    = 600.f;
	ProximityDistance2   = 1200.f;
	StopForPlayerDistance= 350.f;
	CurrentWaypointIndex = 0;
	bIsWaiting           = false;
	bProximityActive     = false;
	bProximityActive2    = false;
	bStoppedForPlayer    = false;
	ProximitySound       = nullptr;
	ProximitySound2      = nullptr;

	ProximitySoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximitySoundComponent"));
	ProximitySoundComponent->SetupAttachment(CollisionBox);
	ProximitySoundComponent->bAutoActivate = false;

	ProximitySoundComponent2 = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximitySoundComponent2"));
	ProximitySoundComponent2->SetupAttachment(CollisionBox);
	ProximitySoundComponent2->bAutoActivate = false;
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

	UpdateProximitySound();

	// Frenar si el jugador esta demasiado cerca
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player)
	{
		float DistToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
		bStoppedForPlayer  = DistToPlayer <= StopForPlayerDistance;
	}

	if (bIsWaiting || bStoppedForPlayer || Waypoints.Num() == 0) return;

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

void AVehicle::UpdateProximitySound()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	// Sonido cercano
	if (ProximitySound && ProximitySoundComponent)
	{
		if (Dist <= ProximityDistance && !bProximityActive)
		{
			ProximitySoundComponent->SetSound(ProximitySound);
			ProximitySoundComponent->Play();
			bProximityActive = true;
		}
		else if (Dist > ProximityDistance && bProximityActive)
		{
			ProximitySoundComponent->Stop();
			bProximityActive = false;
		}
	}

	// Sonido lejano
	if (ProximitySound2 && ProximitySoundComponent2)
	{
		if (Dist <= ProximityDistance2 && !bProximityActive2)
		{
			ProximitySoundComponent2->SetSound(ProximitySound2);
			ProximitySoundComponent2->Play();
			bProximityActive2 = true;
		}
		else if (Dist > ProximityDistance2 && bProximityActive2)
		{
			ProximitySoundComponent2->Stop();
			bProximityActive2 = false;
		}
	}
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
