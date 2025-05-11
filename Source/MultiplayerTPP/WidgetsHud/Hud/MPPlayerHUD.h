// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MPPlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	UPROPERTY()
	class UTexture2D* CrosshairsCenter;

	UPROPERTY()
	UTexture2D* CrosshairsTop;

	UPROPERTY()
	UTexture2D* CrosshairsBottom;

	UPROPERTY()
	UTexture2D* CrosshairsLeft;

	UPROPERTY()
	UTexture2D* CrosshairsRight;


	float CrosshairSpread;
	float Scale;
	FLinearColor CrosshairsColor;
};

UCLASS()
class MULTIPLAYERTPP_API AMPPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:


	virtual void DrawHUD()override;
	void AddPlayerOverlay();
	void AddAnnouncementOverlay();

	//Player Overlay Widget
	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay = nullptr;

	//Announcement Overlay Widget
	UPROPERTY()
	class UAnnouncementOverlay* AnnouncementOverlay = nullptr;

protected:

	virtual void BeginPlay()override;

private:


	//Player Overlay Widget Blueprint Class
	UPROPERTY(EditAnywhere, Category = " Player Overlay")
	TSubclassOf<class UUserWidget> OverlayClass;

	//Announcement Overlay Widget BlueprintClass
	UPROPERTY(EditAnywhere, Category = " Announcement Overlay")
	TSubclassOf<class UUserWidget> AnnouncementOverlayClass;

	UPROPERTY()
		FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, float Scale, FLinearColor Color);

	UPROPERTY(EditAnywhere)
		float CrosshairsCustomSpread = 10.0f;

public:

	//Getters and setters

	FORCEINLINE void SetHUD(const FHUDPackage& Package) { HUDPackage = Package; }
	
};
