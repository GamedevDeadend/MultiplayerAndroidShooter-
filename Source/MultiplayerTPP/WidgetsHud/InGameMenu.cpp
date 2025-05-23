// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
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

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance != nullptr)
	{
		MutiplayerSubsystem = MutiplayerSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MutiplayerSubsystem;
	}

	if (MutiplayerSubsystem != nullptr && MutiplayerSubsystem->MultiplayerOnStartSessionDelegate.IsBound() == false)
	{
		MutiplayerSubsystem->MultiplayerOnStartSessionDelegate.AddDynamic(this, &UInGameMenu::OnDestroySession);
	}
}

void UInGameMenu::OnDestroySession(bool bWasSuccessful)
{
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

void UInGameMenu::ReturnToMainMenu()
{
	UGameInstance* GameInstance = GetGameInstance();

	if (MenuButton != nullptr)
	{
		MenuButton->SetIsEnabled(false);
	}

	if (GameInstance != nullptr)
	{
		MutiplayerSubsystem = MutiplayerSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MutiplayerSubsystem;
	}

	if (MutiplayerSubsystem != nullptr)
	{
		MutiplayerSubsystem->DestroySessions();
	}


}

void UInGameMenu::MenuTeardown()
{
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

	if (MutiplayerSubsystem != nullptr && MutiplayerSubsystem->MultiplayerOnDestroySessionDelegate.IsBound() == true)
	{
		MutiplayerSubsystem->MultiplayerOnDestroySessionDelegate.RemoveDynamic(this, &UInGameMenu::OnDestroySession);
	}
}
