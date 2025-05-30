// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UInGameMenu : public UUserWidget
{
	GENERATED_BODY()

private:

	UPROPERTY()
	class UEOS_VoiceSettings_UI* VoiceChatMenu = nullptr;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class UEOS_VoiceSettings_UI> VoiceChatClass = nullptr;

	APlayerController* PlayerController;

	void OnMatchLeft();

	UPROPERTY(meta = (BindWidget))
	class UButton* MenuButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* VoiceChat;

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MutiplayerSubsystem = nullptr;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void ShowVoiceChat();

public:

	UFUNCTION()
	void ReturnToMainMenu();

	void MenuSetup();
	void MenuTeardown();
	
};
