// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GM.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"

void ADeathMatch_GM::PlayerEliminated(AMPPlayer* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttackingPlayerController)
{
	EliminatedCharacter->Elim();
}
