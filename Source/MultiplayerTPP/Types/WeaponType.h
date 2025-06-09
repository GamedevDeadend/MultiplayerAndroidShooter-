#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AR_Auto UMETA(DisplayName = "Auto Fire"),
	EWT_AR_Burst UMETA(DisplayName = "Burst Fire"),
	EWT_AR_Single UMETA(DisplayName = "Single Fire"),
	EWT_ROCKET UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),

	EWT_MAX UMETA(DisplayName = "MAX STATE"),//For checking max no. of states
};
