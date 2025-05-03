// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void CheckTimeSync(float DeltaTime);
	virtual float GetServerTime();
	void SetHUDTime();

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

	UPROPERTY()
	class AMPPlayerHUD* PlayerHUD = nullptr;

	float MatchTime = 120.0f;
	int32 CountDownInt = 0;

public:

	void SetHUDHealth(float MaxHealth, float CurrentHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmoCount(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponInfo(class UWeaponDataAsset* WeaponDataAsset);
	void SetHUDMatchCountDown(float Sec);

	void ShowDefeatMessage(FString DefeatMessage);
	void HideDefeatMessage();

	virtual void OnPossess(APawn* InPawn) override;

};
