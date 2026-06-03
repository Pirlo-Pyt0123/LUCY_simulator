// Fill out your copyright notice in the Description page of Project Settings.

#include "RiskHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void URiskHUD::NativeConstruct()
{
	Super::NativeConstruct();

	if (TxtLucyStatus) TxtLucyStatus->SetText(FText::FromString(TEXT("LUCY Online")));
	if (RiskBar)       RiskBar->SetPercent(0.f);
	if (TxtRiskLevel)  TxtRiskLevel->SetText(FText::FromString(TEXT("SAFE")));
	if (TxtTitulo)     TxtTitulo->SetText(FText::FromString(TEXT("Iniciando analisis...")));
	if (TxtExplicacion)TxtExplicacion->SetText(FText::FromString(TEXT("")));
}

void URiskHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsCritical || !PanelBorder) return;

	// Pulso sinusoidal en el borde del panel cuando es CRITICAL
	PulseTime += InDeltaTime * 4.f;
	float Alpha = (FMath::Sin(PulseTime) * 0.5f + 0.5f) * 0.5f + 0.1f; // rango 0.1 - 0.6
	PanelBorder->SetBrushColor(FLinearColor(1.f, 0.05f, 0.05f, Alpha));
}

void URiskHUD::UpdateRisk(const FString& RiskLevel, const FString& Titulo, const FString& Explicacion)
{
	float    Percent;
	FSlateColor BarColor;
	FSlateColor TextColor;

	if (RiskLevel == TEXT("CRITICAL"))
	{
		Percent     = 1.0f;
		BarColor    = FSlateColor(FLinearColor(1.f, 0.1f, 0.1f));
		TextColor   = FSlateColor(FLinearColor(1.f, 0.1f, 0.1f));
		bIsCritical = true;
		PulseTime   = 0.f;
	}
	else if (RiskLevel == TEXT("WARNING"))
	{
		Percent     = 0.6f;
		BarColor    = FSlateColor(FLinearColor(1.f, 0.6f, 0.f));
		TextColor   = FSlateColor(FLinearColor(1.f, 0.6f, 0.f));
		bIsCritical = false;
		if (PanelBorder) PanelBorder->SetBrushColor(FLinearColor(1.f, 0.6f, 0.f, 0.15f));
	}
	else // SAFE
	{
		Percent     = 0.2f;
		BarColor    = FSlateColor(FLinearColor(0.1f, 0.9f, 0.2f));
		TextColor   = FSlateColor(FLinearColor(0.1f, 0.9f, 0.2f));
		bIsCritical = false;
		if (PanelBorder) PanelBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
	}

	if (RiskBar)
	{
		RiskBar->SetPercent(Percent);
		RiskBar->SetFillColorAndOpacity(BarColor.GetSpecifiedColor());
	}

	if (TxtRiskLevel)
	{
		TxtRiskLevel->SetText(FText::FromString(RiskLevel));
		TxtRiskLevel->SetColorAndOpacity(TextColor);
	}

	if (TxtTitulo)     TxtTitulo->SetText(FText::FromString(Titulo));
	if (TxtExplicacion)TxtExplicacion->SetText(FText::FromString(Explicacion));
	if (TxtLucyStatus) TxtLucyStatus->SetText(FText::FromString(TEXT("LUCY Analizando...")));
}

void URiskHUD::SetHUDVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
