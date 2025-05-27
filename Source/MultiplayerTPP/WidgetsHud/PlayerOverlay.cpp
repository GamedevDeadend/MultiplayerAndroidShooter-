// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerOverlay.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UPlayerOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (DisplayMessage)
	{
		DisplayMessage->SetText(FText::FromString(""));
		DisplayMessage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerOverlay::ShowHighPingWarning()
{
	if (High_Ping_Icon != nullptr && High_Ping_Anim != nullptr && !IsPlayingAnimation())
	{
		PlayAnimation(High_Ping_Anim,0.0f, 3);
	}
}

void UPlayerOverlay::ShowTeamStats()
{
	if (TeamStats != nullptr)
	{
		TeamStats->SetVisibility(ESlateVisibility::Visible);
	}
}
