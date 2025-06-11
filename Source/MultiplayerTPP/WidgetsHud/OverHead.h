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


	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable)
	void SetDisplayText(FString TextValue);
	//Getters And Setters
public:


};
