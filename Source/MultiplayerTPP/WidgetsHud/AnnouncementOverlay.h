// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API UAnnouncementOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> WarmupTimer;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> MatchInfo;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> MatchAnnouncement;
	
};
