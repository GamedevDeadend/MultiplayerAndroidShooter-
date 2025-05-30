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

	UFUNCTION(BlueprintCallable)
		void MenuSetup();

private:

	class UEOS_Auth_Subsystem* Subsystem;

	class APlayerController* PlayerController = nullptr;
	class IVoiceChatUser* VoiceChatUser = nullptr;

UPROPERTY(meta = (BindWidget))
	class UComboBoxString* InputDevices;

protected: 

};
