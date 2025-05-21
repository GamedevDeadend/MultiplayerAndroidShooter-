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
#include "Kismet/GameplayStatics.h"
#include "MultiplayerTPP/WidgetsHud/AnnouncementOverlay.h"
#include "MultiplayerTPP/PlayerComponents/CombatComponent.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"

/*
* Special naming convention
* Sr_... means variable is from server
* Ctrl_.. means variables from control. This convention is used to distinguish varibales from Game_Mode classes
*/

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<AMPPlayerHUD>(GetHUD());

	ServerCheckMatchState();
}

void AMPPlayerController::PollInit()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

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

	DOREPLIFETIME(AMPPlayerController, Ctrl_MatchState);
}

void AMPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);
	PollInit();

	CheckForLatestPing(DeltaTime);

}

void AMPPlayerController::CheckForLatestPing(float DeltaTime)
{
	HighPingCheckRunningTime += DeltaTime;
	if (HighPingCheckRunningTime > HighPingCheckFrequencyTime || bIsCheckingFirstTime)
	{

		if (PlayerState != nullptr)
		{
			LatestPing = PlayerState->GetPingInMilliseconds();
			SetHUDPing();

			if (LatestPing > HighPingThreshold)
			{
				PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

				if (IsLocalPlayerController())
				{
					ServerCheckHighPing(true);
				}

				if (PlayerHUD != nullptr && PlayerHUD->PlayerOverlay != nullptr)
				{
					PlayerHUD->PlayerOverlay->ShowHighPingWarning();
					bIsCheckingFirstTime = false;
				}

			}
			else
			{
				if (IsLocalPlayerController())
				{
					ServerCheckHighPing(false);
				}
				bIsCheckingFirstTime = false;
			}

			HighPingCheckRunningTime = 0.0f;
		}
	}
}

void AMPPlayerController::ServerCheckHighPing_Implementation(bool bIsPingHigh)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, TEXT("On Server HighPing"));

	if (HighPingDelegate.IsBound() == true)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Purple, TEXT("On Server HighPing Firing"));
		HighPingDelegate.Broadcast(bIsPingHigh);
	}
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
	SingleTripTime = (0.5f * RountTripTime);
	float CurrentServerTime = TimeServerRecievedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

/// <summary>
/// Function which performs calculation for count down timer final HUD values and then pass it to HUD 
/// </summary>
void AMPPlayerController::SetHUDTime()
{
	if (Ctrl_LevelStartTime == 0.0f && HasAuthority())
	{
		ADeathMatch_GM* GameMode = Cast<ADeathMatch_GM>(UGameplayStatics::GetGameMode(this));
		if (GameMode != nullptr)
		{
			Ctrl_LevelStartTime = GameMode->LevelStartTime;
		}
	}


	float TimeLeft = 0.0f;

	if(Ctrl_MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = Ctrl_WarmupTime - (GetServerTime() - Ctrl_LevelStartTime);
	}
	else if (Ctrl_MatchState == MatchState::InProgress)
	{
		TimeLeft = (   Ctrl_MatchTime - (  GetServerTime() - (Ctrl_LevelStartTime + Ctrl_WarmupTime)  )   );
	}
	else if (Ctrl_MatchState == MatchState::Cooldown)
	{
		TimeLeft = Ctrl_CooldownTime - (GetServerTime() - (Ctrl_LevelStartTime + Ctrl_WarmupTime + Ctrl_MatchTime) );
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		DeathMatch_GM = DeathMatch_GM == nullptr ? Cast<ADeathMatch_GM>(UGameplayStatics::GetGameMode(this)) : DeathMatch_GM;
		
		if (DeathMatch_GM != nullptr)
		{
			SecondsLeft = FMath::CeilToInt(DeathMatch_GM->CountDownTime + Ctrl_LevelStartTime);
		}
	}


	if (Ctrl_CountDownInt != SecondsLeft)
	{
		if (Ctrl_MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}

		if (Ctrl_MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);

		}

		if (Ctrl_MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
	}

	Ctrl_CountDownInt = SecondsLeft;
}

void AMPPlayerController::ServerCheckMatchState_Implementation()
{
	ADeathMatch_GM* GameMode = Cast<ADeathMatch_GM>(UGameplayStatics::GetGameMode(this));
	if (GameMode != nullptr)
	{
		Ctrl_WarmupTime = GameMode->WarmupTime;
		Ctrl_MatchTime = GameMode->MatchTime;
		Ctrl_LevelStartTime = GameMode->LevelStartTime;
		Ctrl_MatchState = GameMode->GetMatchState();
		Ctrl_CooldownTime = GameMode->CooldownTime;

		ClientJoinMidGame(Ctrl_MatchState, Ctrl_WarmupTime, Ctrl_MatchTime, Ctrl_LevelStartTime, Ctrl_CooldownTime);
	}

}

void AMPPlayerController::ClientJoinMidGame_Implementation(FName Sr_MatchState, float Sr_WarmupTime, float Sr_MatchTime, float Sr_LevelStartTime, float Sr_Cooldown)
{
		this->Ctrl_WarmupTime = Sr_WarmupTime;
		this->Ctrl_MatchTime = Sr_MatchTime;
		this->Ctrl_LevelStartTime = Sr_LevelStartTime;
		this->Ctrl_MatchState = Sr_MatchState;
		this->Ctrl_CooldownTime = Sr_Cooldown;
		OnMatchStateSet(Sr_MatchState);

		if (Sr_MatchState == MatchState::WaitingToStart && PlayerHUD != nullptr)
		{
			PlayerHUD->AddAnnouncementOverlay();
		}
}

/// <summary>
/// Update Countdown timer on HUD
/// </summary>
/// <param name="Sec"></param>
void AMPPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidPlayerOverlay = PlayerHUD != nullptr && PlayerHUD->PlayerOverlay != nullptr && PlayerHUD->PlayerOverlay->CountDown != nullptr;

	if (bIsValidPlayerOverlay)
	{
		if (CountDownTime < 0.0f)
		{
			PlayerHUD->PlayerOverlay->CountDown->SetText(FText::FromString(""));
			return;
		}

		int32 Minutes = FMath::FloorToInt32(CountDownTime / 60.0f);
		int32 Seconds = CountDownTime - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->PlayerOverlay->CountDown->SetText(FText::FromString(CountDownText));
	}
}

void AMPPlayerController::SetHUDAnnouncementCountDown(float WarmupTime)
{

	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidAnnouncementOverlay = PlayerHUD && PlayerHUD->AnnouncementOverlay && PlayerHUD->AnnouncementOverlay->WarmupTimer;

	if (bIsValidAnnouncementOverlay)
	{

		if (WarmupTime < 0.0f)
		{
			PlayerHUD->AnnouncementOverlay->WarmupTimer->SetText(FText::FromString(""));
			return;
		}

		int32 Minutes = FMath::FloorToInt32(WarmupTime / 60.0f);
		int32 Seconds = WarmupTime - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->AnnouncementOverlay->WarmupTimer->SetText(FText::FromString(CountDownText));
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
		FString PlayerHealth = FString::Printf(TEXT("% d / % d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
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
	Ctrl_MatchState = NewMatchState;

	if (Ctrl_MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (Ctrl_MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}

/// <summary>
/// Rep Notifier for Match State Variable
/// </summary>
void AMPPlayerController::OnRep_MatchState()
{
	if (Ctrl_MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
		CheckForLatestPing(0.0f);
	}
	else if (Ctrl_MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
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

void AMPPlayerController::HandleMatchCooldown()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsHudValid =
		PlayerHUD != nullptr &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->AnnouncementOverlay != nullptr &&
		PlayerHUD->AnnouncementOverlay->MatchAnnouncement &&
		PlayerHUD->AnnouncementOverlay->MatchInfo;

	if (bIsHudValid)
	{
		//HideAnnouncementOverlay();
		PlayerHUD->PlayerOverlay->RemoveFromParent();

		if (PlayerHUD->AnnouncementOverlay != nullptr)
		{
			PlayerHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Start In : ");
			PlayerHUD->AnnouncementOverlay->MatchAnnouncement->SetText(FText::FromString(AnnouncementText));
			ShowWinners();
		}

	}

	AMPPlayer* MPPlayer = Cast<AMPPlayer>(GetPawn());
	if (MPPlayer != nullptr && MPPlayer->GetCombatComponent() != nullptr)
	{
		MPPlayer->bIsGameplayDisabled = true;
		MPPlayer->GetCombatComponent()->FirePressed(false);
		MPPlayer->GetCombatComponent()->SetAiming(false);
	}
}

void AMPPlayerController::ShowWinners()
{
	FString InfoString = "";
	ADeathMatch_GS* GameState = Cast<ADeathMatch_GS>(UGameplayStatics::GetGameState(this));
	if (GameState != nullptr)
	{
		auto TopScoringPlayers = GameState->TopScoringPlayers;
		auto Curr_PlayerState = GetPlayerState<AMPPlayerState>();

		if (TopScoringPlayers.Num() == 0)
		{
			InfoString = "No Winner Is There";
		}
		else if (TopScoringPlayers.Num() == 1 && TopScoringPlayers[0] == Curr_PlayerState)
		{
			InfoString = "You are Winner ;)";
		}
		else if (TopScoringPlayers.Num() == 1)
		{
			InfoString = FString::Printf(TEXT("Winner is \n%s"), *TopScoringPlayers[0]->GetPlayerName());
		}
		else if (TopScoringPlayers.Num() > 1)
		{
			InfoString = "We have a Tie :\n";

			for (auto TiedPlayers : TopScoringPlayers)
			{
				InfoString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayers->GetPlayerName()));
			}
		}


	}

	PlayerHUD->AnnouncementOverlay->MatchInfo->SetText(FText::FromString(InfoString));
}

void AMPPlayerController::SetHUDPing()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidPlayerOverlay = PlayerHUD != nullptr && PlayerHUD->PlayerOverlay != nullptr && PlayerHUD->PlayerOverlay->Ping_Count != nullptr;

	if (bIsValidPlayerOverlay)
	{
		PlayerHUD->PlayerOverlay->Ping_Count->SetText(  FText::FromString( FString::Printf(TEXT("%d ms"), (int32)LatestPing ) )  );
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
