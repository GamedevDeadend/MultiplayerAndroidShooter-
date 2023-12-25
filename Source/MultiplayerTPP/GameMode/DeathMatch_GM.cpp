// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GM.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include"Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"

void ADeathMatch_GM::PlayerEliminated(AMPPlayer* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttackingPlayerController)
{
	if (AttackingPlayerController == nullptr || AttackingPlayerController->PlayerState == nullptr) return;
	if (EliminatedPlayerController == nullptr || EliminatedPlayerController->PlayerState == nullptr) return;

	AMPPlayerState* AttackerPlayerState = AttackingPlayerController ? Cast<AMPPlayerState>(AttackingPlayerController->PlayerState) : nullptr;
	AMPPlayerState* EliminatedPlayerState = EliminatedPlayerController ? Cast<AMPPlayerState>(EliminatedPlayerController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
		
	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeat(1);
	}

	if(EliminatedCharacter)
	EliminatedCharacter->Elim();
}

void ADeathMatch_GM::RequestRespawn(ACharacter* ElimCharacter, AController* ElimPlayerController)
{	
	
	if (ElimCharacter)
	{
		ElimCharacter->Reset();
		ElimCharacter->Destroy();
	}

	if (ElimPlayerController)
	{
		TArray<AActor*> PlayerStarts; 
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomSelection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(ElimPlayerController, PlayerStarts[RandomSelection]);
	}
}
