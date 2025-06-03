    // Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/EditableTextBox.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/ComboBoxString.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


bool UMenuSystem::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (Host != nullptr)
    {
        //Binding Function To Button
        Host->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    }

    if (Join != nullptr)
    {
        //Binding Function To Button
        Join->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
    }

    if (ET_NumOfPlayers != nullptr)
    {
        ET_NumOfPlayers->OnTextChanged.AddDynamic(this, &ThisClass::InputEntered);
    }

    if (Close_MatchSettings != nullptr)
    {
        Close_MatchSettings->OnClicked.AddDynamic(this, &ThisClass::CloseMatchSettings);
    }

    if (CreateButton != nullptr)
    {
        CreateButton->OnClicked.AddDynamic(this, &ThisClass::CreateSessions);
    }

    if (MatchTypes != nullptr)
    {
        MatchTypes->OnSelectionChanged.AddDynamic(this, &ThisClass::GameModeSelected);
    }

    if (FindButton != nullptr)
    {
        FindButton->OnClicked.AddDynamic(this, &ThisClass::StartFindingSessions);
    }

    return true;
}

void UMenuSystem::NativeDestruct()
{
    Menuteardown();
	Super::NativeDestruct();
}

void UMenuSystem :: MenuSetup(FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    //Menu Widget Setup
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;

    UWorld *World = GetWorld();
    if (World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);

        }
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
      MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }
    
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionDelegate.AddUObject(this, &ThisClass::OnFindSession);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionDelegate.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionDelegate.AddDynamic(this, &ThisClass::OnStartSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
    }
}

void UMenuSystem::HostButtonClicked()
{
    Host->SetIsEnabled(false);

    if (MatchSettings != nullptr && MatchSettings->GetVisibility() == ESlateVisibility::Hidden)
    {
        if (FindButton != nullptr)
        {
            FindButton->SetIsEnabled(false);
        }

        MatchSettings->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UMenuSystem::JoinButtonClicked()
{
    Join->SetIsEnabled(false);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked"));
    }

    if (MatchSettings != nullptr && MatchSettings->GetVisibility() == ESlateVisibility::Hidden)
    {
        if (CreateButton != nullptr)
        {
            CreateButton->SetIsEnabled(false);
        }

        MatchSettings->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

void UMenuSystem::InputEntered(const FText& NewText)
{
    bool bIsValid = false;
    FString InputVal = NewText.ToString();

    for (int i = 2; i < 9; i++)
    {
        FString Temp = FString::FromInt(i);
        if (InputVal.Compare(Temp) == 0)
        {
            NumConnections = i;
            MultiplayerSessionsSubsystem->NumOfConnections = NumConnections;
            bIsValid = true;
            Error_Txt->SetText(FText::FromString("Go Ahead...."));
            break;
        }
    }

    if (bIsValid == false)
    {
        if (Error_Txt != nullptr)
        {
            Error_Txt->SetText(FText::FromString("Please Enter Valid Num of Player (2-8)...."));
        }
        return;
    }
}

void UMenuSystem::CloseMatchSettings()
{
    if (MatchSettings != nullptr && MatchSettings->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
    {
        MatchSettings->SetVisibility(ESlateVisibility::Hidden);
        Host->SetIsEnabled(true);
        Join->SetIsEnabled(true);
    }
}

void UMenuSystem::GameModeSelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    MatchType = SelectedItem;
    MultiplayerSessionsSubsystem->MatchType = MatchType;
}

void UMenuSystem::CreateSessions()
{
    if (MultiplayerSessionsSubsystem)
    {
        if (CreateButton != nullptr)
        {
            CreateButton->SetIsEnabled(false);
        }

#if !P2PMODE
        MultiplayerSessionsSubsystem->CreateSession(MatchType);
#endif
        MultiplayerSessionsSubsystem->CreateLobby();
    }
}

void UMenuSystem::StartFindingSessions()
{
    if (FindButton != nullptr)
    {
        FindButton->SetIsEnabled(false);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Starting Finding Sessions"));
    }
    //UE_LOG(LogTemp, Warning, TEXT("JOIN BUTTON CLICKED"));
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
}


void UMenuSystem::Menuteardown()
{
    RemoveFromParent();
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController *PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputMode;
            PlayerController->SetInputMode(InputMode);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}



void UMenuSystem::OnCreateSession(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        //if (GEngine)
        //{
        //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session Was Created Menu Callback"));
        //}

        UWorld* World = GetWorld();
        if (World)
        {
            World->ServerTravel(FString(PathToLobby)); //?listen Opens Level as listen server
        }
    }

    else
    {
        //if (GEngine)
        //{
        //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("Session Creation Failed"));
        //}

        CreateButton->SetIsEnabled(true);
    }
}

void UMenuSystem::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked7"));
    }

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString IPAddress;
            SessionInterface->GetResolvedConnectString(NAME_GameSession, IPAddress);

            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

            if (PlayerController)
            {
                PlayerController->ClientTravel(IPAddress, ETravelType::TRAVEL_Absolute);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage
                    (
                        -1,
                        15.0f,
                        FColor::Green,
                        FString::Printf(TEXT("Connected to IP : %s"), *IPAddress)
                    );
                }
            }
        }
    }

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        Join->SetIsEnabled(true);
    }
}

void UMenuSystem::OnStartSession(bool bWasSuccessful)
{
    if (bWasSuccessful == true)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, FString("Session Has Started and Progression is True"));
        }
    }
}

void UMenuSystem::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuSystem::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
    if (GEngine)
    {
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("JoinButtonClicked4 %d"), SessionResult.Num()));
    }

    for (auto Result : SessionResult)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString("JoinButtonClicked4.1"));
        }

        FString Id = Result.GetSessionIdStr();
        FString User = Result.Session.OwningUserName;
        FString MatchMode;
        Result.Session.SessionSettings.Get(FName("MatchType"), MatchMode);

        if (MatchMode == MatchType)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage
                (
                    -1,
                    15.0f,
                    FColor::Green,
                    FString::Printf(TEXT("User:%s, Id:%s"), *Id, *User)
                );
            }

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage
                (
                    -1,
                    15.0f,
                    FColor::Green,
                    FString::Printf(TEXT("Joined %s Match"), *MatchMode)
                );
            }

            MultiplayerSessionsSubsystem->JoinSessions(Result);
        }
    }

    if (!bWasSuccessful || SessionResult.Num() == 0)
    {
        FindButton->SetIsEnabled(true);
    }
}