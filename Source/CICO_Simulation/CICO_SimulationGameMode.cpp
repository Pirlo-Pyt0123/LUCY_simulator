// Copyright Epic Games, Inc. All Rights Reserved.

#include "CICO_SimulationGameMode.h"
#include "CICO_SimulationCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACICO_SimulationGameMode::ACICO_SimulationGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
