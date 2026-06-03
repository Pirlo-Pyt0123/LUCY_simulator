// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UMediaPlayer;
class UMediaSource;
class USoundBase;

UCLASS()
class CICO_SIMULATION_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// --- Widgets enlazados al Blueprint (nombres exactos en WBP_MainMenu) ---

	// Boton para iniciar el juego
	UPROPERTY(meta = (BindWidget))
	UButton* BtnJugar;

	// Boton para salir del juego
	UPROPERTY(meta = (BindWidget))
	UButton* BtnSalir;

	// Texto del titulo del juego
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtTitulo;

	// Subtitulo debajo del titulo (BindWidgetOptional: no rompe si no existe en BP)
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TxtSubtitulo;

	// Image widget del fondo (asignar textura del MediaPlayer desde editor)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ImgFondo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Video")
	UMediaPlayer* BackgroundPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Video")
	UMediaSource* BackgroundSource;

	// Musica de fondo del menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Audio")
	USoundBase* MenuMusic;

	// Sonido al hacer click en un boton
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Audio")
	USoundBase* ButtonClickSound;

private:
	UFUNCTION()
	void OnJugarClicked();

	UFUNCTION()
	void OnSalirClicked();
};
