// Copyright Epic Games, Inc. All Rights Reserved.

#include "CICO_SimulationGameMode.h"
#include "CICO_SimulationCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ACICO_SimulationGameMode::ACICO_SimulationGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;

	}

	
}

void ACICO_SimulationGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Restaurar input del jugador al cargar el nivel de juego
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor       = false;
		PC->bEnableClickEvents     = false;
		PC->bEnableMouseOverEvents = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}
