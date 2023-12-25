// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerOverlay.h"
#include "Components/TextBlock.h"

void UPlayerOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (DisplayMessage)
	{
		DisplayMessage->SetText(FText::FromString(""));
		DisplayMessage->SetVisibility(ESlateVisibility::Hidden);
	}
}
