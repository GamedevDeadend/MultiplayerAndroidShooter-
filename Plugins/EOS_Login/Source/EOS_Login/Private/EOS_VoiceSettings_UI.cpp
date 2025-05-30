// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_VoiceSettings_UI.h"
#include "EOS_Auth_Subsystem.h"
#include "VoiceChat.h"
#include "Components/ComboBoxString.h"

void UEOS_VoiceSettings_UI::MenuSetup()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Menu Setup Called"));
	AddToViewport(5);
	SetVisibility(ESlateVisibility::Visible);

	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<APlayerController>(World->GetFirstPlayerController()) : PlayerController;
	}

	if (PlayerController != nullptr)
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetWidgetToFocus(TakeWidget());
		PlayerController->SetInputMode(InputModeGameAndUI);
	}

	if (World != nullptr)
	{
		auto GameInstance = World->GetGameInstance();
		Subsystem = GameInstance->GetSubsystem<UEOS_Auth_Subsystem>();
		if (Subsystem != nullptr)
		{
			VoiceChatUser  = Subsystem->GetLocalPlayerChatInterface();

			if (VoiceChatUser != nullptr)
			{
				for (auto Device : VoiceChatUser->GetAvailableInputDeviceInfos())
				{
					InputDevices->AddOption(Device.DisplayName);
				}

				InputDevices->SetSelectedOption(VoiceChatUser->GetInputDeviceInfo().DisplayName);
			}
		}
	}
}
