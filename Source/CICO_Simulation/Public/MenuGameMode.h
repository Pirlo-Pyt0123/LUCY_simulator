// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

class UMainMenu;


UCLASS()
class CICO_SIMULATION_API AMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMenuGameMode();

	// Clase del widget de menu (asignar WBP_MainMenu en el editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UMainMenu> MainMenuClass;

protected:
	virtual void BeginPlay() override;

private:
	UMainMenu* MainMenuInstance;
};
