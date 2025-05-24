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

	APlayerController* PlayerController;

	void OnMatchLeft();

	UPROPERTY(meta = (BindWidget))
	class UButton* MenuButton;

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MutiplayerSubsystem = nullptr;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

public:

	UFUNCTION()
	void ReturnToMainMenu();

	void MenuSetup();
	void MenuTeardown();
	
};
