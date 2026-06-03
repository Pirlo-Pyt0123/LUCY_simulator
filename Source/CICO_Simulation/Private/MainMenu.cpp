// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// Titulo del proyecto
	if (TxtTitulo)
		TxtTitulo->SetText(FText::FromString(TEXT("CICO Simulation")));

	if (TxtSubtitulo)
		TxtSubtitulo->SetText(FText::FromString(TEXT("Road Safety Simulation")));

	// Reproducir video de fondo en loop
	if (BackgroundPlayer && BackgroundSource)
	{
		BackgroundPlayer->SetLooping(true);
		BackgroundPlayer->OpenSource(BackgroundSource);
		BackgroundPlayer->Play();
	}

	// Enlazar botones a sus funciones
	if (BtnJugar)
	{
		BtnJugar->OnClicked.AddDynamic(this, &UMainMenu::OnJugarClicked);
	}

	if (BtnSalir)
	{
		BtnSalir->OnClicked.AddDynamic(this, &UMainMenu::OnSalirClicked);
	}
}

void UMainMenu::OnJugarClicked()
{
	// Cargar el nivel principal — cambia "MainLevel" por el nombre real de tu nivel
	UGameplayStatics::OpenLevel(this, FName("ThirdPersonMap"));
}

void UMainMenu::OnSalirClicked()
{
	// Salir del juego
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
