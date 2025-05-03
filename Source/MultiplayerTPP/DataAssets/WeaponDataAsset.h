// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MultiplayerTPP/Types/WeaponType.h"
#include "WeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

private : 

	//TODO: Declutter Weapon Class and Add all Weapon Data here
	UPROPERTY(EditAnywhere)
		FString WeaponName;

	UPROPERTY(EditAnywhere)  
		UTexture2D* WeaponIcon;

public:
	//Getters
	FORCEINLINE FString GetWeaponName()const { return WeaponName; }
	FORCEINLINE UTexture2D* GetWeaponIcon()const { return WeaponIcon; }
};
