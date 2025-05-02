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

private:

	UPROPERTY()
	class AMPPlayerHUD* PlayerHUD = nullptr;

public:

	void SetHUDHealth(float MaxHealth, float CurrentHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmoCount(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponInfo(class UWeaponDataAsset* WeaponDataAsset);

	void ShowDefeatMessage(FString DefeatMessage);
	void HideDefeatMessage();

	virtual void OnPossess(APawn* InPawn) override;

};
