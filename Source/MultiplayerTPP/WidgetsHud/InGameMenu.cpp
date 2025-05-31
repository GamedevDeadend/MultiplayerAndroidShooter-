// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "EOS_VoiceSettings_UI.h"
#include "GameFrameWork/GameModeBase.h"


void UInGameMenu::MenuSetup()
{
	AddToViewport();

	UWorld* World = GetWorld();
	bIsFocusable = true;


	if (World != nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<APlayerController>(World->GetFirstPlayerController()) : PlayerController;
	}

	if (PlayerController != nullptr)
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetWidgetToFocus(TakeWidget());
		PlayerController->SetInputMode(InputModeGameAndUI);
	}

	if (MenuButton != nullptr)
	{
		MenuButton->OnClicked.AddDynamic(this, &UInGameMenu::ReturnToMainMenu);
	}

	if (VoiceChat != nullptr)
	{
		VoiceChat->OnClicked.AddDynamic(this, &UInGameMenu::ShowVoiceChat);
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance != nullptr)
	{
		MutiplayerSubsystem = MutiplayerSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MutiplayerSubsystem;
	}

	if (MutiplayerSubsystem != nullptr)
	{
		MutiplayerSubsystem->MultiplayerOnDestroySessionDelegate.AddDynamic(this, &UInGameMenu::OnDestroySession);
	}
}


void UInGameMenu::OnDestroySession(bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Destroy Successful"));

	if (!bWasSuccessful)
	{
		MenuButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();

		if (GameMode != nullptr)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;

			if(PlayerController != nullptr)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason( FText::FromString("Gracefull Exit") );
			}
		}
	}
}

void UInGameMenu::ShowVoiceChat()
{

	if (VoiceChatMenu != nullptr)
	{
		if (VoiceChatMenu->GetVisibility() == ESlateVisibility::Hidden)
		{
			VoiceChatMenu->SetVisibility(ESlateVisibility::Visible);
			VoiceChatMenu->MenuSetup();
		}
	}
}

void UInGameMenu::ReturnToMainMenu()
{
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Return To Main Menu"));

	if (MenuButton != nullptr)
	{
		MenuButton->SetIsEnabled(false);
	}

	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<APlayerController>(World->GetFirstPlayerController()) : PlayerController;
		AMPPlayer* Player = Cast<AMPPlayer>(PlayerController->GetPawn());

		if (Player != nullptr)
		{
			Player->OnLeavingMatch.AddUObject(this, &UInGameMenu::OnMatchLeft);
			AMPPlayerState* PlayerState = Cast<AMPPlayerState>(Player->GetPlayerState());
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Return To Main Menu Calling ServerLeaveGame"));
			Player->ServerLeaveGame();
		}
	}
}

void UInGameMenu::MenuTeardown()
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, TEXT("On Show In  Game Menu TearDown"));
	RemoveFromParent();

	UWorld* World = GetWorld();
	bIsFocusable = false;


	if (World != nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<APlayerController>(World->GetFirstPlayerController()) : PlayerController;
	}

	if (PlayerController != nullptr)
	{
		FInputModeGameOnly InputModeGameOnly;
		PlayerController->SetInputMode(InputModeGameOnly);
	}

	if (MenuButton != nullptr && MenuButton->OnClicked.IsBound() == true)
	{
		MenuButton->OnClicked.RemoveDynamic(this, &UInGameMenu::ReturnToMainMenu);
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance != nullptr)
	{
		MutiplayerSubsystem = MutiplayerSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MutiplayerSubsystem;
	}

	if (MutiplayerSubsystem != nullptr)
	{
		MutiplayerSubsystem->MultiplayerOnDestroySessionDelegate.RemoveDynamic(this, &UInGameMenu::OnDestroySession);
	}

	if (VoiceChat != nullptr)
	{
		VoiceChat->OnClicked.RemoveDynamic(this, &UInGameMenu::ShowVoiceChat);
	}
}
void UInGameMenu::OnMatchLeft()
{
	UGameInstance* GameInstance = GetGameInstance();
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Reached On Match Left"));


	if (GameInstance != nullptr)
	{
		MutiplayerSubsystem = MutiplayerSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MutiplayerSubsystem;
	}

	if (MutiplayerSubsystem != nullptr)
	{
		MutiplayerSubsystem->DestroySessions();
	}
}
