// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch_GM.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include"Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "MultiplayerSessionsSubsystem.h"


namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}


ADeathMatch_GM::ADeathMatch_GM()
{
	bDelayedStart = true;
}

void ADeathMatch_GM::RegisterAllPlayers()
{
	Curr_GameState = Curr_GameState == nullptr ? GetGameState<ADeathMatch_GS>() : Curr_GameState;

	auto AvailablePlayers = GetWorld()->GetGameState()->PlayerArray;

	for (auto PlayerState : AvailablePlayers)
	{
		AMPPlayerState* NewPlayerState = Cast<AMPPlayerState>(PlayerState);
		if (NewPlayerState != nullptr)
		{
			Curr_GameState->AddNewPlayer(NewPlayerState);
		}
	}
}

void ADeathMatch_GM::KickPlayer(AMPPlayerState* PlayerState)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("On KickPlayer"));

	if (PlayerState != nullptr)
	{
		Curr_GameState = Curr_GameState == nullptr ? GetGameState<ADeathMatch_GS>() : Curr_GameState;

		if(Curr_GameState != nullptr && Curr_GameState->TopScoringPlayers.Contains(PlayerState) == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Top Player Updated"));
			Curr_GameState->TopScoringPlayers.Remove(PlayerState);
		}

		AMPPlayer* LeavingPlayer = Cast<AMPPlayer>(PlayerState->GetPawn());

		if (LeavingPlayer != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Called ElimPlayer"));
			LeavingPlayer->Elim(true);
		}
	}
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
		CountDownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartTime);
		if (CountDownTime <= 0.0f)
		{
			RegisterAllPlayers();
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = MatchTime - ( GetWorld()->GetTimeSeconds() - (LevelStartTime + WarmupTime));
		if (CountDownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime - (GetWorld()->GetTimeSeconds() - (LevelStartTime + WarmupTime + MatchTime));
		if (CountDownTime <= 0.0f)
		{

			SetMatchState(MatchState::WaitingPostMatch);
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
	Curr_GameState = Curr_GameState == nullptr ? GetGameState<ADeathMatch_GS>() : Curr_GameState;


	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState && Curr_GameState != nullptr)
	{
		AttackerPlayerState->AddToScore(1.0f);
		Curr_GameState->UpdateTopScore(AttackerPlayerState);
		Curr_GameState->UpdatePlayersInfo(AttackerPlayerState);
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
