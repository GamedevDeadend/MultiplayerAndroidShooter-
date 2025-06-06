// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerOverlay.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"

void UPlayerOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (DisplayMessage)
	{
		DisplayMessage->SetText(FText::FromString(""));
		DisplayMessage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (All_Speaker != nullptr)
	{
		All_Speaker->OnClicked.AddDynamic(this, &ThisClass::OnAllSpeakerClicked);
	}

	if (All_Mic != nullptr)
	{
		All_Mic->OnClicked.AddDynamic(this, &ThisClass::OnAllMicClicked);
	}
}

void UPlayerOverlay::NativeDestruct()
{

	Super::NativeDestruct();
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

void UPlayerOverlay::OnAllSpeakerClicked()
{
	if (All_Speaker->GetRenderOpacity() == 0.5f)
	{
		All_Speaker->SetRenderOpacity(1.0f);
	}
	else
	{
		All_Speaker->SetRenderOpacity(0.5f);
	}

	OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(GetOwningPlayer()) : OwnerController;
	OwnerController->Toggle_Speaker_All();
}

void UPlayerOverlay::OnAllMicClicked()
{
	if (All_Mic->GetRenderOpacity() == 0.5f)
	{
		All_Mic->SetRenderOpacity(1.0f);
	}
	else
	{
		All_Mic->SetRenderOpacity(0.5f);
	}

	OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(GetOwningPlayer()) : OwnerController;
	OwnerController->Toggle_Mic_All();
}
