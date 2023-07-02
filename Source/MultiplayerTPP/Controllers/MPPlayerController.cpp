// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"
#include "MultiplayerTPP/WidgetsHud/Hud/MPPlayerHUD.h"
#include "MultiplayerTPP/WidgetsHud/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<AMPPlayerHUD>(GetHUD());
}

void AMPPlayerController::SetHUDHealth(float MaxHealth, float CurrentHealth)
{
	PlayerHUD = nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->HealthBar && PlayerHUD->PlayerOverlay->HealthText;

	if (bIsValidPlayerOverlay)
	{
		PlayerHUD->PlayerOverlay->HealthBar->SetPercent(CurrentHealth / MaxHealth);
		FString PlayerHealth = FString::Printf(TEXT("Health: % d / % d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
		PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(PlayerHealth));
	}
}
