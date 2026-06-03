// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RiskHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;

UCLASS()
class CICO_SIMULATION_API URiskHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lucy|HUD")
	void UpdateRisk(const FString& RiskLevel, const FString& Titulo, const FString& Explicacion);

	UFUNCTION(BlueprintCallable, Category = "Lucy|HUD")
	void SetHUDVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- Widgets (nombres exactos en WBP_RiskHUD) ---
	UPROPERTY(meta = (BindWidget))
	UProgressBar* RiskBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtRiskLevel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtTitulo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtExplicacion;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtLucyStatus;

	// Border principal del panel (BindWidgetOptional: no rompe si no existe en BP)
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* PanelBorder;

private:
	bool bIsCritical = false;
	float PulseTime  = 0.f;
};
