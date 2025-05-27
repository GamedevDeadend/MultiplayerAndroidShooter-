// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamDeathMatch_GS.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATeamDeathMatch_GS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATeamDeathMatch_GS, RedTeamScore);
	DOREPLIFETIME(ATeamDeathMatch_GS, BlueTeamScore);
}

void ATeamDeathMatch_GS::SetRedTeamScore()
{
	RedTeamScore++;

	UWorld* World = GetWorld();

	if (World == nullptr) return;

	AMPPlayerController* Controller = World->GetFirstPlayerController<AMPPlayerController>();

	if (Controller != nullptr)
	{
		Controller->SetRedTeamScore(RedTeamScore);
	}
}

void ATeamDeathMatch_GS::SetBlueTeamScore()
{
	BlueTeamScore++;

	UWorld* World = GetWorld();

	if (World == nullptr) return;

	AMPPlayerController* Controller = World->GetFirstPlayerController<AMPPlayerController>();

	if (Controller != nullptr)
	{
		Controller->SetBlueTeamScore(BlueTeamScore);
	}
}

void ATeamDeathMatch_GS::OnRep_RedTeamScore()
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	AMPPlayerController* Controller = World->GetFirstPlayerController<AMPPlayerController>();

	if (Controller != nullptr)
	{
		Controller->SetRedTeamScore(RedTeamScore);
	}
}

void ATeamDeathMatch_GS::OnRep_BlueTeamScore()
{
	UWorld* World = GetWorld();

	if (World == nullptr) return;

	AMPPlayerController* Controller = World->GetFirstPlayerController<AMPPlayerController>();

	if (Controller != nullptr)
	{
		Controller->SetBlueTeamScore(BlueTeamScore);
	}
}

