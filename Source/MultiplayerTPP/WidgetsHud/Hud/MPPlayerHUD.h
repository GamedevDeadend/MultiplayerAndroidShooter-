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

	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	UTexture2D* CrosshairsLeft;
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

	//Health Widget
	class UPlayerOverlay* PlayerOverlay;

protected:

	virtual void BeginPlay()override;

private:

	void AddPlayerOverlay();

	//Health Widget Blueprint Class
	UPROPERTY(EditAnywhere, Category = " Player Overlay")
	TSubclassOf<class UUserWidget> OverlayClass;


	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, float Scale, FLinearColor Color);

	UPROPERTY(EditAnywhere)
		float CrosshairsCustomSpread = 10.0f;

public:

	//Getters and setters

	FORCEINLINE void SetHUD(const FHUDPackage& Package) { HUDPackage = Package; }
	
};
