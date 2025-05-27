#pragma once

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	EGM_SDM UMETA(DisplayName = "Solo Death Match"),
	EGM_TDM UMETA(DisplayName = "Team Death Match"),

	EGM_MAX UMETA(DisplayName = "MAXGAME MODE")

};