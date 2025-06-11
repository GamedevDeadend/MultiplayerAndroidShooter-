// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/EditableText.h"

#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "MultiplayerTPP/WidgetsHud/Hud/MPPlayerHUD.h"
#include "MultiplayerTPP/WidgetsHud/PlayerOverlay.h"
#include "MultiplayerTPP/DataAssets/WeaponDataAsset.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/GameMode/DeathMatch_GM.h"
#include "MultiplayerTPP/WidgetsHud/AnnouncementOverlay.h"
#include "MultiplayerTPP/PlayerComponents/CombatComponent.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"
#include "MultiplayerTPP/GameInstance/Multiplayer_GI.h"
#include "MultiplayerTPP/WidgetsHud/InGameMenu.h"
#include "MultiplayerSessionsSubsystem.h"

#include "MultiplayerTPP/GameStates/TeamDeathMatch_GS.h"
#include "EOS_Auth_Subsystem.h"
#include "VoiceChat.h"

/*
* Special naming convention
* Sr_... means variable is from server
* Ctrl_.. means variables from control. This convention is used to distinguish varibales from Game_Mode classes
*/


void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/*
* This code is for testing Only
*/
	//Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;

	//if (Curr_GI != nullptr)
	//{
	//	Curr_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
	//}

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

void AMPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindAction("ShowMenu", IE_Pressed, this, &AMPPlayerController::ShowInGameMenu);
		InputComponent->BindAction("ShowScoreBoard", IE_Pressed, this, &AMPPlayerController::ShowPlayersStats);
		InputComponent->BindAction("ShowScoreBoard", IE_Released, this, &AMPPlayerController::HidePlayersStats);
		InputComponent->BindAction("ShowAllChat", IE_Pressed, this, &AMPPlayerController::ToggleShowAllChat);

		Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;

		if(Curr_GI->CurrentGameModeType == EGameModeType::EGM_TDM)
		{
			InputComponent->BindAction("ShowTeamChat", IE_Pressed, this, &AMPPlayerController::ToggleShowTeamChat);
		}
	}
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
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, TEXT("On Server HighPing"));

	if (HighPingDelegate.IsBound() == true)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Purple, TEXT("On Server HighPing Firing"));
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

void AMPPlayerController::ServerToggleTeamVoiceChat_Implementation(const FString& Name, bool bShouldMute, EPlayerTeam Team)
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, TEXT("Now Calling Multicast to Mute Non team Players"));
	//MulticastToggleTeamVoiceChat(Name, bShouldMute, Team);
}

void AMPPlayerController::MulticastToggleTeamVoiceChat_Implementation(const FString& Name, bool bShouldMute, EPlayerTeam Team)
{
	//Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;
	//VoiceSubsystem = VoiceSubsystem == nullptr ? Curr_GI->GetSubsystem<UEOS_VoiceAuth_Subsystem>() : VoiceSubsystem;
	//VoiceChatUser = VoiceChatUser == nullptr ? VoiceSubsystem->GetLocalPlayerChatInterface() : VoiceChatUser;
	//TDM_GS = TDM_GS == nullptr ? GetWorld()->GetGameState<ATeamDeathMatch_GS>() : TDM_GS;
	//AMPPlayerState* CurrentPlayerState = GetPlayerState<AMPPlayerState>();

	//if (CurrentPlayerState->GetPlayerTeam() != Team)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, TEXT("Muting Non team Players"));
	//	if (bShouldMute == true)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player To %s Off"), *Name));
	//		VoiceChatUser->SetPlayerMuted(Name, true);
	//	}
	//	else
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player To %s On"), *Name));
	//		VoiceChatUser->SetPlayerMuted(Name, false);
	//	}

	//}
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

		FString ScoreAmt = FString::Printf(TEXT("%02d"), FMath::FloorToInt(Score));
		PlayerHUD->PlayerOverlay->ScoreAmt->SetText(FText::FromString(ScoreAmt));
	}
	else
	{
		bIsPlayerOverlayIntialized = true;
		Cached_Score = Score;
	}
}

/// <summary>
/// Update Red Team Score in TDM
/// </summary>
/// <param name="NewScore"></param>
void AMPPlayerController::SetRedTeamScore(float NewScore)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;


	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->RedTeamScore;
	//bool bIsValidPlayerOverlayMessage = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DisplayMessage;

	if (bIsValidPlayerOverlay)
	{
		//if (bIsValidPlayerOverlayMessage != false && Score == 0.0f)
		//{
		//	PlayerHUD->PlayerOverlay->DisplayMessage->SetVisibility(ESlateVisibility::Visible);
		//}

		FString ScoreAmt = FString::Printf(TEXT("%02d"), FMath::FloorToInt(NewScore));
		PlayerHUD->PlayerOverlay->RedTeamScore->SetText(FText::FromString(ScoreAmt));
	}
}

void AMPPlayerController::SetBlueTeamScore(float NewScore)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;


	bool bIsValidPlayerOverlay = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->BlueTeamScore;
	//bool bIsValidPlayerOverlayMessage = PlayerHUD && PlayerHUD->PlayerOverlay && PlayerHUD->PlayerOverlay->DisplayMessage;

	if (bIsValidPlayerOverlay)
	{
		//if (bIsValidPlayerOverlayMessage != false && Score == 0.0f)
		//{
		//	PlayerHUD->PlayerOverlay->DisplayMessage->SetVisibility(ESlateVisibility::Visible);
		//}

		FString ScoreAmt = FString::Printf(TEXT("%02d"), FMath::FloorToInt(NewScore));
		PlayerHUD->PlayerOverlay->BlueTeamScore->SetText(FText::FromString(ScoreAmt));
	}
}

void AMPPlayerController::ServerSetChatMessage_Implementation(const FText& PlayerName, const FText& PlayerMsg, const EPlayerTeam Team)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "InsideServerRPC ServerSetChatMessage_Implementation");

	ADeathMatch_GS* GameState = Cast<ADeathMatch_GS>(UGameplayStatics::GetGameState(GetWorld()));


	GameState->ChatMessageToSend.PlayerMsg = PlayerMsg;
	GameState->ChatMessageToSend.PlayerName = PlayerName;
	GameState->ChatMessageToSend.SendingPlayerTeam = Team;

	if (GEngine->IsEditor() || GEngine->GetNetMode(GetWorld()) == NM_ListenServer)
	{
		if (IsLocalPlayerController())
		{
			AMPPlayerState* MPPlayerState = Cast<AMPPlayerState>(PlayerState);
			if (!MPPlayerState) return;

			if (GameState->ChatMessageToSend.SendingPlayerTeam == EPlayerTeam::EPT_NONE ||
				GameState->ChatMessageToSend.SendingPlayerTeam == MPPlayerState->GetPlayerTeam())
			{
				SetChatMessage(GameState->ChatMessageToSend.PlayerName, GameState->ChatMessageToSend.PlayerMsg, GameState->ChatMessageToSend.SendingPlayerTeam);
			}
		}
	}
}

void AMPPlayerController::SetChatMessage(const FText& ChatMessagePlayerName, const FText& ChatMessage, const EPlayerTeam& MsgSendingPlayerTeam)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Setting chat msg on widget");

	AMPPlayerState* MPPlayerState = GetPlayerState<AMPPlayerState>();

	if (MsgSendingPlayerTeam != MPPlayerState->GetPlayerTeam()) {return;}

	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidPlayerOverlay =
		PlayerHUD != nullptr &&
		PlayerHUD->PlayerOverlay != nullptr &&
		PlayerHUD->PlayerOverlay->MsgPlayerName != nullptr &&
		PlayerHUD->PlayerOverlay->Msg_Txt != nullptr;



	if (bIsValidPlayerOverlay == true)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Msg is set"));

		PlayerHUD->PlayerOverlay->MsgPlayerName->SetText(ChatMessagePlayerName);
		PlayerHUD->PlayerOverlay->Msg_Txt->SetText(ChatMessage);
	}


    GetWorldTimerManager().SetTimer
	(
		TimerHandle, 
		[this]() 
		{
			PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
			bool bIsValidPlayerOverlay
				= PlayerHUD != nullptr &&
				PlayerHUD->PlayerOverlay != nullptr &&
				PlayerHUD->PlayerOverlay->MsgPlayerName &&
				PlayerHUD->PlayerOverlay->Msg_Txt != nullptr;

			if (bIsValidPlayerOverlay == true)
			{
				PlayerHUD->PlayerOverlay->MsgPlayerName->SetText(FText::FromString(""));
				PlayerHUD->PlayerOverlay->Msg_Txt->SetText(FText::FromString(""));
			}

			GetWorldTimerManager().ClearTimer(TimerHandle);
		},
		5.0f,
		false
	);
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
		FString DefeatAmt = FString::Printf(TEXT("%02d"), Defeats);
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
		FString AmmoTxt = FString::Printf(TEXT("%02d"), Ammo);
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
		FString AmmoTxt = FString::Printf(TEXT("%02d"), Ammo);
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
	else if (Ctrl_MatchState == MatchState::WaitingPostMatch)
	{
		HandlePostMatch();
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
	else if (Ctrl_MatchState == MatchState::WaitingPostMatch)
	{
		HandlePostMatch();
	}
}

void AMPPlayerController::HandleMatchHasStarted()
{
		PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

		if (PlayerHUD != nullptr)
		{
			HideAnnouncementOverlay();
			PlayerHUD->AddPlayerOverlay();

			Curr_GI = GetGameInstance<UMultiplayer_GI>();


			FString MapName = GetWorld()->GetMapName();
			if (MapName.Compare("SoloDeathMatch") == 0)
			{
				Curr_GI->CurrentGameModeType = EGameModeType::EGM_SDM;
			}
			else if (MapName.Compare("TeamDeathMatch") == 0)
			{
				if (Curr_GI != nullptr)
				{
					Curr_GI->CurrentGameModeType = EGameModeType::EGM_TDM;
				}
			}



			if (PlayerHUD != nullptr && Curr_GI != nullptr && Curr_GI->CurrentGameModeType == EGameModeType::EGM_TDM)
			{
				PlayerHUD->PlayerOverlay->ShowTeamStats();
			}
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
			FString AnnouncementText("Returning To Menu In : ");
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

void AMPPlayerController::HandlePostMatch()
{
	Subsystem = Subsystem == nullptr ? GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>() : Subsystem;

	if (Subsystem != nullptr)
	{
		Subsystem->MultiplayerOnDestroySessionDelegate.AddDynamic(this, &AMPPlayerController::OnDestroySession);
		Subsystem->DestroySessions();
	}
}

void AMPPlayerController::ShowWinners()
{
	FString InfoString = "";
	Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;

	//FString PlayerScores = "";

	if (Curr_GI != nullptr)
	{
		switch (Curr_GI->CurrentGameModeType)
		{
			case EGameModeType::EGM_SDM :
			{
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
			}
			break;

		case EGameModeType::EGM_TDM:
			{
				TDM_GS = TDM_GS == nullptr ? Cast<ATeamDeathMatch_GS>(UGameplayStatics::GetGameState(this)) : TDM_GS;
				if (TDM_GS != nullptr)
				{
					auto RedTeamScore = TDM_GS->GetRedTeamScore();
					auto BlueTeamScore = TDM_GS->GetBlueTeamScore();


					if (BlueTeamScore == RedTeamScore)
					{
						InfoString = "No Winner Is There";
					}
					else if (RedTeamScore > BlueTeamScore)
					{
						InfoString = "Red Team Is Winner ;)";
					}
					else
					{
						InfoString = "Blue Team Is Winner ;)";
					}
				}

				//SetScoreBoardString(TDM_GS, PlayerScores);
			}
			break;
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

void AMPPlayerController::ShowPlayersStats()
{
	//if (IsLocalController() == false) return;

	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		ADeathMatch_GS* DM_GameState = Cast<ADeathMatch_GS>(World->GetGameState());
		Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;

		FString ScoreBoardString = "";
		SetScoreBoardString(DM_GameState, ScoreBoardString);

		PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

		bool bIsValidPlayerOverlay = 
			(
				PlayerHUD != nullptr &&
				PlayerHUD->PlayerOverlay != nullptr &&
				PlayerHUD->PlayerOverlay->ScoreBoardText != nullptr &&
				PlayerHUD->PlayerOverlay->ScoreBoardBorder != nullptr
			);

		if (bIsValidPlayerOverlay)
		{
			PlayerHUD->PlayerOverlay->ScoreBoardBorder->SetVisibility(ESlateVisibility::Visible);
			PlayerHUD->PlayerOverlay->ScoreBoardText->SetVisibility(ESlateVisibility::Visible);
			PlayerHUD->PlayerOverlay->ScoreBoardText->SetText(FText::FromString(ScoreBoardString));
		}
	}
}

void AMPPlayerController::SetScoreBoardString(ADeathMatch_GS* DM_GameState, FString& ScoreBoardString)
{
	if (DM_GameState != nullptr && Curr_GI != nullptr)
	{
		switch (Curr_GI->CurrentGameModeType)
		{
			case EGameModeType::EGM_SDM:
			{
				ScoreBoardString = "PLAYERS =>\n";
				for (auto& Stat : DM_GameState->PlayersInfo)
				{
					ScoreBoardString.Append(FString::Printf(TEXT("%s\t%02d\n"), *Stat.PlayerName, (int32)Stat.CurrScore));
				}
			}
			break;

			case EGameModeType::EGM_TDM:
			{
				ScoreBoardString = "SCOREBOARD ;)\n";
				ScoreBoardString.Append(FString("Team Red\n"));
				for (auto& Stat : DM_GameState->PlayersInfo)
				{
					if (Stat.PlayerTeam == EPlayerTeam::EPT_RED)
					{
						ScoreBoardString.Append(FString::Printf(TEXT("%s\t%02d\n"), *Stat.PlayerName, (int32)Stat.CurrScore));
					}
				}

				ScoreBoardString.Append(FString("\n"));
				ScoreBoardString.Append(FString("Team Blue\n"));
				for (auto& Stat : DM_GameState->PlayersInfo)
				{
					if (Stat.PlayerTeam == EPlayerTeam::EPT_BLUE)
					{
						ScoreBoardString.Append(FString::Printf(TEXT("%s\t%02d\n"), *Stat.PlayerName, (int32)Stat.CurrScore));
					}
				}
			}

			break;
		}
	}
}

void AMPPlayerController::HidePlayersStats()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;
	bool bIsValidPlayerOverlay =
		(
			PlayerHUD != nullptr &&
			PlayerHUD->PlayerOverlay != nullptr &&
			PlayerHUD->PlayerOverlay->ScoreBoardText != nullptr &&
			PlayerHUD->PlayerOverlay->ScoreBoardBorder != nullptr
			);

	if (bIsValidPlayerOverlay)
	{
		PlayerHUD->PlayerOverlay->ScoreBoardBorder->SetVisibility(ESlateVisibility::Hidden);
		PlayerHUD->PlayerOverlay->ScoreBoardText->SetVisibility(ESlateVisibility::Hidden);
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

void AMPPlayerController::OnDestroySession(bool bWasSuccess)
{
	if (!bWasSuccess)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();

		if (GameMode != nullptr)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			if (this != nullptr)
			{
				ClientReturnToMainMenuWithTextReason(FText::FromString("Gracefull Exit"));
			}
		}
	}
}

void AMPPlayerController::ToggleShowAllChat()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay =
		PlayerHUD != nullptr &&
		PlayerHUD->PlayerOverlay != nullptr &&
		PlayerHUD->PlayerOverlay->Chat_Box != nullptr;

	if (bIsAllChatVisible)
	{
		FInputModeGameOnly InputModeData;
		SetInputMode(InputModeData);
		//SetShowMouseCursor(false);

		PlayerHUD->PlayerOverlay->Chat_Box->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		PlayerHUD->PlayerOverlay->Chat_Box->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(PlayerHUD->PlayerOverlay->Chat_Box->TakeWidget());
		//SetShowMouseCursor(true);
		SetInputMode(InputModeData);
	}
	bIsAllChatVisible = !bIsAllChatVisible;
}

void AMPPlayerController::ToggleShowTeamChat()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<AMPPlayerHUD>(GetHUD()) : PlayerHUD;

	bool bIsValidPlayerOverlay =
		PlayerHUD != nullptr &&
		PlayerHUD->PlayerOverlay != nullptr&&
		PlayerHUD->PlayerOverlay->Chat_Box != nullptr;


	if (bIsTeamChatVisible)
	{
		FInputModeGameOnly InputModeData;
		SetInputMode(InputModeData);

		PlayerHUD->PlayerOverlay->bIsTeamModeChat = false;
		PlayerHUD->PlayerOverlay->Chat_Box->SetVisibility(ESlateVisibility::Hidden);

	}
	else
	{
		PlayerHUD->PlayerOverlay->bIsTeamModeChat = true;
		PlayerHUD->PlayerOverlay->Chat_Box->SetVisibility(ESlateVisibility::Visible);

		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(PlayerHUD->PlayerOverlay->Chat_Box->TakeWidget());
		SetInputMode(InputModeData);
	}

	bIsTeamChatVisible = !bIsTeamChatVisible;
}

void AMPPlayerController::ShowInGameMenu()
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, TEXT("On Show In  Game Menu"));

	if (InGameMenuClass == nullptr) return;

	if (InGameMenu == nullptr)
	{
		InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
	}

	if (InGameMenu != nullptr)
	{
		bIsInGameMenu = !bIsInGameMenu;

		if (bIsInGameMenu)
		{
			InGameMenu->MenuSetup();
		}
		else
		{
			InGameMenu->MenuTeardown();
		}
	}

}

void AMPPlayerController::Toggle_Speaker_All()
{
	Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;
	VoiceSubsystem = VoiceSubsystem == nullptr ? Curr_GI->GetSubsystem<UEOS_VoiceAuth_Subsystem>() : VoiceSubsystem;
	VoiceChatUser = VoiceChatUser == nullptr ? VoiceSubsystem->GetLocalPlayerChatInterface() : VoiceChatUser;

	if(VoiceChatUser == nullptr || VoiceSubsystem == nullptr || Curr_GI == nullptr)
	{
		return;
	}

	if( bIsAllSpeakerSwitchedOff == true)
	{
		for (auto& Participants : VoiceChatUser->GetPlayersInChannel(VoiceSubsystem->GetCurrentChannel()))
		{
			if (Participants != VoiceChatUser->GetLoggedInPlayerName())
			{
				VoiceChatUser->SetPlayerMuted(Participants, false);
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player %s is Unmuted"), *Participants));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player %s is not Unmuted"), *Participants));
			}
		}

		bIsAllSpeakerSwitchedOff = false;
		
	}
	else
	{

		for (auto& Participants : VoiceChatUser->GetPlayersInChannel(VoiceSubsystem->GetCurrentChannel()))
		{
			if (Participants != VoiceChatUser->GetLoggedInPlayerName())
			{
				VoiceChatUser->SetPlayerMuted(Participants, true);
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player %s is muted"), *Participants));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Player %s is not muted"), *Participants));
			}
		}

		bIsAllSpeakerSwitchedOff = true;
	}


}

void AMPPlayerController::Toggle_Mic_All()
{
	Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;
	VoiceSubsystem = VoiceSubsystem == nullptr ? Curr_GI->GetSubsystem<UEOS_VoiceAuth_Subsystem>() : VoiceSubsystem;
	VoiceChatUser = VoiceChatUser == nullptr ? VoiceSubsystem->GetLocalPlayerChatInterface() : VoiceChatUser;

	if (bIsMicToAllSwitchedOff == true)
	{
		VoiceChatUser->TransmitToAllChannels();
		bIsMicToAllSwitchedOff = false;
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Mic is Unmuted")));
	}
	else
	{
		VoiceChatUser->TransmitToNoChannels();
		bIsMicToAllSwitchedOff = true;
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Mic is Muted")));
	}
}

void AMPPlayerController::ToggleVoiceMode()
{
	//if (bIsAllSpeakerSwitchedOff == true)
	//{
	//	Toggle_Speaker_All();
	//}

	//if( bIsMicToAllSwitchedOff == true)
	//{
	//	Toggle_Mic_All();
	//}


	//Curr_GI = Curr_GI == nullptr ? GetGameInstance<UMultiplayer_GI>() : Curr_GI;
	//VoiceSubsystem = VoiceSubsystem == nullptr ? Curr_GI->GetSubsystem<UEOS_VoiceAuth_Subsystem>() : VoiceSubsystem;
	//VoiceChatUser = VoiceChatUser == nullptr ? VoiceSubsystem->GetLocalPlayerChatInterface() : VoiceChatUser;
	//TDM_GS = TDM_GS == nullptr ? GetWorld()->GetGameState<ATeamDeathMatch_GS>() : TDM_GS;
	//AMPPlayerState* CurrentPlayerState = GetPlayerState<AMPPlayerState>();



	//if (bIsTeamVoiceChat == false)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Sending Toggle VoiceMode Request To Server Currently All")));
	//
	//	ServerToggleTeamVoiceChat(CurrentPlayerState->GetPlayerId(), true, CurrentPlayerState->GetPlayerTeam());
	//	bIsTeamVoiceChat = true;

	//	for (auto& Participants : TDM_GS->PlayersInfo)
	//	{
	//		if (Participants.PlayerTeam != CurrentPlayerState->GetPlayerTeam())
	//		{
	//			VoiceChatUser->SetPlayerMuted(Participants.PlayerName, true);
	//			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s to Player Off"), *Participants.PlayerName));
	//		}
	//	}
	//}
	//else
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("Sending Toggle VoiceMode Request To Server Currently Team")));
	//	ServerToggleTeamVoiceChat(CurrentPlayerState->GetPlayerId(), false, CurrentPlayerState->GetPlayerTeam());
	//	bIsTeamVoiceChat = false;

	//	for (auto& Participants : TDM_GS->PlayersInfo)
	//	{
	//		if (Participants.PlayerTeam != CurrentPlayerState->GetPlayerTeam())
	//		{
	//			VoiceChatUser->SetPlayerMuted(Participants.PlayerName, false);
	//			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, FString::Printf(TEXT("%s to Player On"), *Participants.PlayerName));
	//		}
	//	}
	//}


}
