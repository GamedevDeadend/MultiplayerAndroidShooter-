// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS_VoiceSettings_UI.h"
#include "EOS_Auth_Subsystem.h"
#include "VoiceChat.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/Slider.h"

void UEOS_VoiceSettings_UI::MenuSetup()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Menu Setup Called"));

	if (InputVolume == nullptr || OutputDevices == nullptr || OutputVolume == nullptr || InputDevices == nullptr || Back == nullptr)
	{
		/*
		* Components are not Binded
		*/
		return;
	}

	InputDevices->OnSelectionChanged.AddDynamic(this, &ThisClass::OnInputDeviceChanged);
	OutputDevices->OnSelectionChanged.AddDynamic(this, &ThisClass::OnOutputDeviceChanged);
	Back->OnClicked.AddDynamic(this, &ThisClass::HideMenu);
	InputVolume->OnValueChanged.AddDynamic(this, &ThisClass::SetInputVolume);
	OutputVolume->OnValueChanged.AddDynamic(this, &ThisClass::SetOutputVolume);


	UWorld* World = GetWorld();

	if (World != nullptr)
	{
		PlayerController = PlayerController == nullptr ? Cast<APlayerController>(World->GetFirstPlayerController()) : PlayerController;
	}

	if (World != nullptr)
	{
		auto GameInstance = World->GetGameInstance();
		Subsystem = GameInstance->GetSubsystem<UEOS_VoiceAuth_Subsystem>();
		if (Subsystem != nullptr)
		{
			VoiceChatUser  = Subsystem->GetLocalPlayerChatInterface();

			if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
			{
				InputDevices->ClearOptions();

				for (auto Device : VoiceChatUser->GetAvailableInputDeviceInfos())
				{
					InputDevices->AddOption(Device.DisplayName);
				}

				InputDevices->SetSelectedOption(VoiceChatUser->GetInputDeviceInfo().DisplayName);


				OutputDevices->ClearOptions();

				for (auto Device : VoiceChatUser->GetAvailableOutputDeviceInfos())
				{
					OutputDevices->AddOption(Device.DisplayName);
				}

				OutputDevices->SetSelectedOption(VoiceChatUser->GetOutputDeviceInfo().DisplayName);
			}
		}
	}
}

void UEOS_VoiceSettings_UI::HideMenu()
{
	SetVisibility(ESlateVisibility::Hidden);

	InputDevices->OnSelectionChanged.RemoveDynamic(this, &ThisClass::OnInputDeviceChanged);
	OutputDevices->OnSelectionChanged.RemoveDynamic(this, &ThisClass::OnOutputDeviceChanged);
	InputVolume->OnValueChanged.RemoveDynamic(this, &ThisClass::SetInputVolume);
	OutputVolume->OnValueChanged.RemoveDynamic(this, &ThisClass::SetOutputVolume);
	Back->OnClicked.RemoveDynamic(this, &ThisClass::HideMenu);
	
}

void UEOS_VoiceSettings_UI::OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (InputDevices != nullptr)
	{
		if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
		{
			for (auto Device : VoiceChatUser->GetAvailableInputDeviceInfos())
			{
				if (Device.DisplayName == SelectedItem)
				{
					VoiceChatUser->SetInputDeviceId(Device.Id);
					OutputVolume->SetValue(VoiceChatUser->GetAudioInputVolume());
				}
			}

		}
	}
}

void UEOS_VoiceSettings_UI::OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (OutputDevices != nullptr)
	{
		if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
		{
			for (auto Device : VoiceChatUser->GetAvailableOutputDeviceInfos())
			{
				if (Device.DisplayName == SelectedItem)
				{
					VoiceChatUser->SetOutputDeviceId(Device.Id);
					OutputVolume->SetValue(VoiceChatUser->GetAudioOutputVolume());
				}
			}

		}
	}
}

void UEOS_VoiceSettings_UI::SetInputVolume(float NewVolume)
{
	if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
	{
		VoiceChatUser->SetAudioInputVolume(NewVolume);
	}
}

void UEOS_VoiceSettings_UI::SetOutputVolume(float NewVolume)
{
	if (VoiceChatUser != nullptr && VoiceChatUser->IsLoggedIn())
	{
		VoiceChatUser->SetAudioOutputVolume(NewVolume);
	}
}
