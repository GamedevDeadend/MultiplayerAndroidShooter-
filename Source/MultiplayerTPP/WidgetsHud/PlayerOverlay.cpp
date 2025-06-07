// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerOverlay.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/EditableText.h"
#include "Animation/WidgetAnimation.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "Input/Events.h"

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

	if(Chat_Box != nullptr)
	{
		Chat_Box->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatBoxTextCommitted);
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

void UPlayerOverlay::OnChatBoxTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (GEngine == nullptr) return;

	if (CommitMethod == ETextCommit::OnEnter && !Text.IsEmpty())
	{
		GameState = GameState == nullptr ? Cast<ADeathMatch_GS>(GetWorld()->GetGameState()) : GameState;
		MPPlayerState = MPPlayerState == nullptr ? Cast<AMPPlayerState>(GetOwningPlayer()->PlayerState) : MPPlayerState;
		OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(GetOwningPlayer()) : OwnerController;

		if (bIsTeamModeChat == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Team Chat: " + Text.ToString());
			OwnerController->ServerSetChatMessage(FText::FromString(MPPlayerState->GetPlayerName()), Text, MPPlayerState->GetPlayerTeam());
			OwnerController->ToggleShowTeamChat();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "All Chat: " + Text.ToString());
			OwnerController->ServerSetChatMessage(FText::FromString(MPPlayerState->GetPlayerName()), Text, EPlayerTeam::EPT_NONE);
			OwnerController->ToggleShowAllChat();
		}

		Chat_Box->SetText(FText::FromString(""));
	}
}

void UPlayerOverlay::NativeDestruct()
{

	if (All_Speaker != nullptr && All_Speaker->OnClicked.IsBound() == true)
	{
		All_Speaker->OnClicked.RemoveDynamic(this, &ThisClass::OnAllSpeakerClicked);
	}

	if (All_Mic != nullptr && All_Mic->OnClicked.IsBound() == true)
	{
		All_Mic->OnClicked.RemoveDynamic(this, &ThisClass::OnAllMicClicked);
	}

	Super::NativeDestruct();
}
