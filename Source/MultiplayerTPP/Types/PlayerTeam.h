#pragma once

UENUM(BlueprintType)
enum class EPlayerTeam : uint8
{
	EPT_RED UMETA(DisplayName = "RED"),
	EPT_BLUE UMETA(DisplayName = "BLUE"),

	EPT_MAX UMETA(DisplayName = "NO TEAM"),//For checking max no. of states
};