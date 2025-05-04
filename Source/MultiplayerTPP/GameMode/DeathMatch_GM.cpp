// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GM.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include"Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"

ADeathMatch_GM::ADeathMatch_GM()
{
	bDelayedStart = true;
}

void ADeathMatch_GM::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void ADeathMatch_GM::Tick(float DeltaTime)
{
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime += WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartTime);
		if (CountDownTime <= 0.0f)
		{
			StartMatch();
		}
	}
}

void ADeathMatch_GM::PlayerEliminated(AMPPlayer* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttackingPlayerController)
{

	if (AttackingPlayerController == nullptr || AttackingPlayerController->PlayerState == nullptr) return;
	if (EliminatedPlayerController == nullptr || EliminatedPlayerController->PlayerState == nullptr) return;

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,FString("Inside Player Eliminated"));
	//}


	AMPPlayerState* AttackerPlayerState = AttackingPlayerController ? Cast<AMPPlayerState>(AttackingPlayerController->PlayerState) : nullptr;
	AMPPlayerState* EliminatedPlayerState = EliminatedPlayerController ? Cast<AMPPlayerState>(EliminatedPlayerController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.0f);
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

void ADeathMatch_GM::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AMPPlayerController* MPPlayerController = Cast<AMPPlayerController>(*It);
		if (MPPlayerController != nullptr)
		{
			MPPlayerController->OnMatchStateSet(MatchState);
		}
	}
}
