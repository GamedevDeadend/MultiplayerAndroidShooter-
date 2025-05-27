// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UPlayerOverlay : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct()override;

public:

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> ScoreAmt = nullptr;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> DefeatAmt;

	///Display Message Text
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> DisplayMessage = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> AmmoCount = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> CarriedAmmoCount = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> WeaponName = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UImage> WeaponIcon = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> CountDown = nullptr;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UImage> High_Ping_Icon = nullptr;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
		TObjectPtr<class UWidgetAnimation> High_Ping_Anim = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> Ping_Count = nullptr;


	void ShowHighPingWarning();
	void ShowTeamStats();
	/*
	* Widgets for TDM
	*/

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> RedTeamScore = nullptr;

	UPROPERTY(meta = (BingWidget))
		TObjectPtr<class UTextBlock> BlueTeamScore = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox>  TeamStats = nullptr;
		


	
};
