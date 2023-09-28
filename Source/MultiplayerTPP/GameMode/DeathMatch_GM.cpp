// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GM.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include"Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ADeathMatch_GM::PlayerEliminated(AMPPlayer* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttackingPlayerController)
{
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
		//ElimPlayerController->;

		TArray<AActor*> PlayerStarts; 
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomSelection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(ElimPlayerController, PlayerStarts[RandomSelection]);
	}
}
