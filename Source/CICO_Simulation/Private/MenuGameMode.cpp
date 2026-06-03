// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuGameMode.h"
#include "MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMenuGameMode::AMenuGameMode()
{
	MainMenuInstance = nullptr;
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuClass)
	{
		MainMenuInstance = CreateWidget<UMainMenu>(GetWorld(), MainMenuClass);
		if (MainMenuInstance)
		{
			MainMenuInstance->AddToViewport();

			// Mostrar cursor y habilitar interaccion con el menu
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (PC)
			{
				PC->bShowMouseCursor        = true;
				PC->bEnableClickEvents      = true;
				PC->bEnableMouseOverEvents  = true;
				PC->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}
