// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "MultiplayerTPP/WidgetsHud/Hud/MPPlayerHUD.h"
#include "MultiplayerTPP/WidgetsHud/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerTPP/DataAssets/WeaponDataAsset.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/GameMode/DeathMatch_GM.h"
#include "MultiplayerTPP/WidgetsHud/AnnouncementOverlay.h"

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<AMPPlayerHUD>(GetHUD());

	if (PlayerHUD != nullptr)
	{
		PlayerHUD->AddAnnouncementOverlay();
	}
}

void AMPPlayerController::PollInit()
{
	if (PlayerOverlay == nullptr)
	{
		if (PlayerHUD != nullptr && PlayerHUD->PlayerOverlay != nullptr)
		{
			PlayerOverlay = PlayerHUD->PlayerOverlay;

			if (PlayerOverlay != nullptr)
			{
				SetHUDHealth(Cached_MaxHealth, Cached_Health);
				SetHUDScore(Cached_Score);
				SetHUDDefeats(Cached_Defeats);
			}
		}
	}
}

void AMPPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerController, MatchState);
}

void AMPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);
	PollInit();

}

/// <summary>
/// Function for periodically calculating CSD for propery syncing
/// </summary>
/// <param name="DeltaTime"></param>
void AMPPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

/// <summary>
/// Getter for getting server time with clientServerDelta taken in consideration
/// </summary>
/// <returns></returns>
float AMPPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

/// <summary>
/// Earliest Method available where we can sync clocks for first time.
/// </summary>
void AMPPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

/// <summary>
/// RPC to Server for tequesting server time
/// </summary>
/// <param name="TimeOfClientRequest"></param>
void AMPPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

/// <summary>
/// RPC from Server to client to give TimeOfClient Request, Time Server Recieved request.
/// These values help in calculation of RTT, CST, CSD.
/// </summary>
/// <param name="TimeOfClientRequest"></param>
/// <param name="TimeServerRecievedClientRequest"></param>
void AMPPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	float RountTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecievedClientRequest + (0.5f * RountTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

/// <summary>
/// Function which performs calculation for count down timer final HUD values and then pass it to HUD 
/// </summary>
void AMPPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	if (CountDownInt != SecondsLeft)
	{
		SetHUDMatchCountDown(MatchTime - GetServerTime());
	}

	CountDownInt = SecondsLeft;
}

/// <summary>
/// Update Countdown timer on HUD
/// </summary>
/// <param name="Sec"></param>
void AMPPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->CountDown;

	if (bIsValidPlayerOverlay)
	{
		int32 Minutes = FMath::FloorToInt32(CountDownTime / 60.0f);
		int32 Seconds = CountDownTime - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->PlayerOverlay->CountDown->SetText(FText::FromString(CountDownText));
	}
}

void AMPPlayerController::HideAnnouncementOverlay()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidAnnouncementOverlay = PlayerHUD && PlayerHUD->AnnouncementOverlay;

	if (bIsValidAnnouncementOverlay)
	{
		PlayerHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
	}

}

/// <summary>
/// Set Max Health and Current Health in Player HUD
/// </summary>
/// <param name="MaxHealth"></param>
/// <param name="CurrentHealth"></param>
void AMPPlayerController::SetHUDHealth(float MaxHealth, float CurrentHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->HealthBar && PlayerHUD->PlayerOverlay->HealthText;

	if (bIsValidPlayerOverlay)
	{
		PlayerHUD->PlayerOverlay->HealthBar->SetPercent(CurrentHealth / MaxHealth);
		FString PlayerHealth = FString::Printf(TEXT("Health: % d / % d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
		PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(PlayerHealth));
	}
	else
	{
		bIsPlayerOverlayIntialized = true;
		Cached_MaxHealth = MaxHealth;
		Cached_Health = CurrentHealth;
	}
}

/// <summary>
/// Set Score or Kills done by Player to HUD
/// </summary>
/// <param name="Score"></param>
void AMPPlayerController::SetHUDScore(float Score)
{

	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;


	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->ScoreAmt;
	//bool bIsValidPlayerOverlayMessage = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DisplayMessage;

	if (bIsValidPlayerOverlay)
	{
		//if (bIsValidPlayerOverlayMessage != false && Score == 0.0f)
		//{
		//	PlayerHUD->PlayerOverlay->DisplayMessage->SetVisibility(ESlateVisibility::Visible);
		//}

		FString ScoreAmt = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerHUD->PlayerOverlay->ScoreAmt->SetText(FText::FromString(ScoreAmt));
	}
	else
	{
		bIsPlayerOverlayIntialized = true;
		Cached_Score = Score;
	}
}

/// <summary>
/// Function to Set Defeats on HUD
/// </summary>
/// <param name="Defeats"></param>
void AMPPlayerController::SetHUDDefeats(int32 Defeats)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;


	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DefeatAmt;

	if (bIsValidPlayerOverlay)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inside Final Call"));
		FString DefeatAmt = FString::Printf(TEXT("%d"), Defeats);
		PlayerHUD->PlayerOverlay->DefeatAmt->SetText(FText::FromString(DefeatAmt));
	}
	else
	{
		bIsPlayerOverlayIntialized = true;
		Cached_Defeats = Defeats;
	}
}

/// <summary>
/// Function to Update Ammount in Player Overlay through HUD
/// </summary>
/// <param name="Ammo"></param>
void AMPPlayerController::SetHUDAmmoCount(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->AmmoCount;

	if (bIsValidPlayerOverlay)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Valid Overlay Ammo Count"));
		//}
		FString AmmoTxt = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->PlayerOverlay->AmmoCount->SetText(FText::FromString(AmmoTxt));
	}

}

/// <summary>
/// Function to update Current Weapon Ammo carried by player
/// </summary>
/// <param name="Ammo"></param>
void AMPPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->CarriedAmmoCount;

	if (bIsValidPlayerOverlay)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Valid Overlay Ammo Count"));
		//}
		FString AmmoTxt = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->PlayerOverlay->CarriedAmmoCount->SetText(FText::FromString(AmmoTxt));
	}
}

/// <summary>
/// Function to Set HUD Weapon Info like name and Image
/// </summary>
/// <param name="WeaponDataAsset"></param>
void AMPPlayerController::SetHUDWeaponInfo(UWeaponDataAsset* WeaponDataAsset)
{
	if (WeaponDataAsset == nullptr) { return; }

	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidWeaponName = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->WeaponName;
	bool bIsValidWeaponIcon = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->WeaponIcon;

	if (bIsValidWeaponName && bIsValidWeaponIcon)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Valid Overlay"));
		//}
		PlayerHUD->PlayerOverlay->WeaponName->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->PlayerOverlay->WeaponName->SetText(FText::FromString(WeaponDataAsset->GetWeaponName()));

		PlayerHUD->PlayerOverlay->WeaponIcon->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->PlayerOverlay->WeaponIcon->SetBrushFromTexture(WeaponDataAsset->GetWeaponIcon(), false);
	}
}


/// <summary>
/// Show Defeat Message When Player is Dead
/// </summary>
/// <param name="DefeatMessage"></param>
void AMPPlayerController::ShowDefeatMessage(FString DefeatMessage)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;


	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DisplayMessage;

	if (bIsValidPlayerOverlay)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Valid Overlay"));
		//}
		PlayerHUD->PlayerOverlay->DisplayMessage->SetVisibility(ESlateVisibility::Visible);
		PlayerHUD->PlayerOverlay->DisplayMessage->SetText(FText::FromString(DefeatMessage));
	}
}

/// <summary>
/// Hides Player Defeat Message after message is shown
/// </summary>
void AMPPlayerController::HideDefeatMessage()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DisplayMessage;

	if (bIsValidPlayerOverlay)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inside Final Call"));
		PlayerHUD->PlayerOverlay->DisplayMessage->SetVisibility(ESlateVisibility::Hidden);
		PlayerHUD->PlayerOverlay->DisplayMessage->SetText(FText::FromString(""));
	}
}

/// <summary>
/// Function to set Matchstate in player controller
/// </summary>
/// <param name="NewMatchState"></param>
void AMPPlayerController::OnMatchStateSet(FName NewMatchState)
{
	MatchState = NewMatchState;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

/// <summary>
/// Rep Notifier for Match State Variable
/// </summary>
void AMPPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AMPPlayerController::HandleMatchHasStarted()
{
		PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

		if (PlayerHUD != nullptr)
		{
			HideAnnouncementOverlay();
			PlayerHUD->AddPlayerOverlay();
		}

}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMPPlayer* InPlayer = Cast<AMPPlayer>(InPawn);

	if (Player)
	{
		SetHUDHealth(InPlayer->GetMaxHealth(), InPlayer->GetHealth());

		auto MPPlayerState = Cast<AMPPlayerState>(this->PlayerState);

		if (MPPlayerState)
		{
			MPPlayerState->SetCanReplicateDefeat(false);
		}
	}

}
