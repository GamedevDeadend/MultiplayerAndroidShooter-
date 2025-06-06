// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EOS_VoiceSettings_UI.generated.h"

/**
 * 
 */
UCLASS()
class EOS_LOGIN_API UEOS_VoiceSettings_UI : public UUserWidget
{
	GENERATED_BODY()

public : 

	UFUNCTION()
		void MenuSetup();

	//UFUNCTION(BlueprintCallable)
	//	void SwitchToTeamVoiceChat();


private:

	UFUNCTION()
		void HideMenu();

	UFUNCTION()
		void OnInputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
		void OnOutputDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
		void SetInputVolume(float NewVolume);

	UFUNCTION()
		void SetOutputVolume(float NewVolume);


	class UEOS_VoiceAuth_Subsystem* Subsystem = nullptr;
	class APlayerController* PlayerController = nullptr;
	class IVoiceChatUser* VoiceChatUser = nullptr;

UPROPERTY(meta = (BindWidget))
	class UComboBoxString* InputDevices;

UPROPERTY(meta = (BindWidget))
	class UComboBoxString* OutputDevices;

UPROPERTY(meta = (BindWidget))
	class USlider* InputVolume;

UPROPERTY(meta = (BindWidget))
	class USlider* OutputVolume;



UPROPERTY(meta = (BindWidget))
	class UButton* Back;

protected: 

};
