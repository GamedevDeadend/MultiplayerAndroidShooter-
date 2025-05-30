// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Delegates/Delegate.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class MULTIPLAYERTPP_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	 FHighPingDelegate HighPingDelegate;

	float SingleTripTime = 0.0f;

	void HideAnnouncementOverlay();

	void ShowDefeatMessage(FString DefeatMessage);

	void HideDefeatMessage();

	void OnMatchStateSet(FName NewMatchState);

	void ShowInGameMenu();

protected:

	virtual void SetupInputComponent()override;
	virtual void BeginPlay() override;
	void PollInit();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	void CheckForLatestPing(float DeltaTime);
	void CheckTimeSync(float DeltaTime);
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName MatchState, float WarmupTime, float MatchTime, float LevelStartTime, float Cooldown);
	

	/*
	* Sync time between Client and Server
	*/

	//Request Current Server Time Passing Client's Time when Request was send
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

	virtual void ReceivedPlayer() override; //Sync with server clock as soon as possible

	//Difference between Server and Client Data
	float ClientServerDelta = 0.0f;


	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.0f;

	float TimeSyncRunningTime = 0.0f;

private:

	bool bIsInGameMenu = false;

	UPROPERTY()
		class UMultiplayer_GI* Curr_GI = nullptr;

	UPROPERTY()
		class UMultiplayerSessionsSubsystem* Subsystem = nullptr;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class UInGameMenu> InGameMenuClass;

	UPROPERTY()
		class UInGameMenu* InGameMenu = nullptr;

	UPROPERTY()
		class AMPPlayerHUD* PlayerHUD = nullptr;

	/// <summary>
	/// Only exists on server
	/// </summary>
	UPROPERTY()
		class ADeathMatch_GM* DeathMatch_GM = nullptr;

	UPROPERTY()
		class UPlayerOverlay* PlayerOverlay = nullptr;

	/*
	* Cached Values
	*/
	bool bIsPlayerOverlayIntialized = false;
	bool bIsCheckingFirstTime = true;

	float Cached_Health = 0.0f;
	float Cached_MaxHealth = 0.0f;
	float Cached_Score = 0.0f;
	int32 Cached_Defeats = 0;

	float Ctrl_LevelStartTime = 0.0f;
	float Ctrl_WarmupTime = 0.0f;
	float Ctrl_MatchTime = 0.0f;
	float Ctrl_CooldownTime = 0.0f;
	int32 Ctrl_CountDownInt = 0;

	/*
	* Ping Showing Variables
	*/

	float LatestPing = 0.0f;
	float HighPingCheckRunningTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Lag Settings")
		float HighPingThreshold = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Lag Settings")
		float HighPingCheckFrequencyTime = 30.0f;



	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName Ctrl_MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	void HandleMatchHasStarted();
	void HandleMatchCooldown();
	void HandlePostMatch();

	void ShowWinners();
	void SetHUDPing();
	void ShowPlayersStats();
	void SetScoreBoardString(class ADeathMatch_GS* DM_GameState, FString& ScoreBoardString);
	void HidePlayersStats();

	UFUNCTION()
	void OnDestroySession(bool bWasSuccess);

	UFUNCTION(Server, Reliable)
	void ServerCheckHighPing(bool bIsPingHigh);

public:


	/*
	* Setters
	*/

	void SetHUDHealth(float MaxHealth, float CurrentHealth);

	void SetHUDScore(float Score);

	void SetHUDDefeats(int32 Defeats);

	void SetHUDAmmoCount(int32 Ammo);

	void SetHUDCarriedAmmo(int32 Ammo);

	void SetHUDWeaponInfo(class UWeaponDataAsset* WeaponDataAsset);

	void SetHUDMatchCountDown(float Sec);

	void SetHUDAnnouncementCountDown(float time);

	void SetRedTeamScore(float NewScore);

	void SetBlueTeamScore(float NewScore);

	/*
	* Getters
	*/
	virtual float GetServerTime();
};
