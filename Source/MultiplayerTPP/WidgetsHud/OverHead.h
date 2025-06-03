// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHead.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERTPP_API UOverHead : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* DisplayText;

protected:

	void SetDisplayText(FString TextValue);

	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* PlayerPawn);

	//Getters And Setters
public:


};
