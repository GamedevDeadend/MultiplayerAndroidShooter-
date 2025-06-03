// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYER_API UMenuSystem : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
		void MenuSetup(FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;


private:

	int32 NumConnections = 2;
	FString MatchType = "FreeForAll";
	FString PathToLobby;

	/*
	* MatchSettings Pannel
	*/

	UPROPERTY(meta = (BindWidget))
		class UButton* Close_MatchSettings;

	UPROPERTY(meta = (BindWidget))
		class UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* FindButton;

	UPROPERTY(meta = (BindWidget))
		class UOverlay* MatchSettings;

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* Error_Txt;

	UPROPERTY(meta = (BindWidget))
		class UEditableTextBox* ET_NumOfPlayers;

	UPROPERTY(meta = (BindWidget))
		class UComboBoxString* MatchTypes;

	UFUNCTION()
		void InputEntered(const FText& NewText);

	UFUNCTION()
		void CloseMatchSettings();

	UFUNCTION()
		void CreateSessions();

	UFUNCTION()
		void StartFindingSessions();

	UFUNCTION()
		void GameModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType);


	/*
	* Start Menu
	*/
	UPROPERTY(meta = (BindWidget))
		class UButton *Join;

	UPROPERTY(meta = (BindWidget))
		UButton *Host;

	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	UFUNCTION()
		void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
		void OnStartSession(bool bWasSuccessful);
	
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);

		void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
		void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);


	void Menuteardown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

};
