#include "MPPlayerState.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

/// <summary>
/// Function to Register Replicating Variable using DOREPLIFETIME
/// </summary>
/// <param name="OutLifetimeProps"></param>
void AMPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerState, DefeatsAmt);
}

/// <summary>
/// Local Function used to increase player score locally
/// </summary>
/// <param name="Amt"></param>
void AMPPlayerState::AddToScore(float Amt)
{
	SetScore(GetScore() + Amt);
	UpdateHUDScore();
}

/// <summary>
/// Rep Notifier for Score Replication
/// </summary>
void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	UpdateHUDScore();
}

/// <summary>
/// Local Function used to update Defeats of Player Locally
/// </summary>
/// <param name="Defeats"></param>
void AMPPlayerState::AddToDefeat(int32 Defeats)
{
	DefeatsAmt += Defeats;
	UpdateHUDDefeats();
}

/// <summary>
/// Rep Notifier for Defeat Replication
/// </summary>
void AMPPlayerState::OnRep_Defeats()
{
	if (CanReplicateDefeat == false)
	{
		UpdateHUDDefeats();
		CanReplicateDefeat = true;
	}
}

/// <summary>
/// Update HUD Score used on both Local and Replicated Huds
/// Ensures Controlller Safety as well
/// </summary>
void AMPPlayerState::UpdateHUDScore()
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
		else
		{
			// Retry after 0.1 seconds if Controller not yet ready
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::UpdateHUDScore, 0.1f, false);
		}
	}
}

/// <summary>
/// Update HUD Defeat used on both Local and Replicated Huds
/// Ensures Controlller Safety as well
/// Displays lossing message as well
/// </summary>
void AMPPlayerState::UpdateHUDDefeats()
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(DefeatsAmt);

			if (DefeatsAmt != 0)
			{
				DisplayLossingMessage();
			}
		}
		else
		{
			// Retry after 0.1 seconds if Controller not yet ready
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::UpdateHUDDefeats, 0.1f, false);
		}
	}
}

/// <summary>
/// Function to display Lossing message
/// </summary>
void AMPPlayerState::DisplayLossingMessage()
{
	if (!Character)
	{
		Character = Cast<AMPPlayer>(GetPawn());
	}

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->ShowDefeatMessage(DisplayMessage);

			// Auto-hide message after 0.5 seconds
			FTimerHandle HideDelayTimerHandle;
			GetWorldTimerManager().SetTimer(HideDelayTimerHandle, this, &ThisClass::HideLossingMessage, 0.5f, false);
		}
	}
}

/// <summary>
/// Hide Lossing Message
/// </summary>
void AMPPlayerState::HideLossingMessage()
{
	if (!Character)
	{
		Character = Cast<AMPPlayer>(GetPawn());
	}

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->HideDefeatMessage();
		}
	}
}


