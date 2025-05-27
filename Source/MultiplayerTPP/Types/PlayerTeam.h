#pragma once

UENUM(BlueprintType)
enum class EPlayerTeam : uint8
{
	EPT_RED UMETA(DisplayName = "RED"),
	EPT_BLUE UMETA(DisplayName = "BLUE"),
	EPT_NONE UMETA(DisplayName = "NO TEAM"),


	EPT_MAX UMETA(DisplayName = "DEFAULT MAX")//For checking max no. of states
};