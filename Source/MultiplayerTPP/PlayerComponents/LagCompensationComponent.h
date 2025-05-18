// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY();

public:

	UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY();

public:

	UPROPERTY()
		float Time;

	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfoMap;
};

USTRUCT(BlueprintType)
struct FHitResult_SSR
{
	GENERATED_BODY();

public:

	UPROPERTY()
	bool bIsHit = false;

	UPROPERTY()
	bool bIsHeadShot = false;
};





UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTPP_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	friend class AMPPlayer;

	ULagCompensationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BuildFrameHistory();
	FHitResult_SSR ServerSideRewind(class AMPPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

protected:
	virtual void BeginPlay() override;

private: 

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
		float MaxFrameHistoryTime = 4.0f;

	UPROPERTY()
		class AMPPlayer* Character;

	UPROPERTY()
		class AMPPlayerController* CharacterController;

		void SaveFramePackage(FFramePackage& Package);

		void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

		void EnableCharacterMeshCollision(AMPPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

		FFramePackage InterpFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

		FHitResult_SSR ConfirmHit_SSR(const FFramePackage& FrameToCheck, AMPPlayer* HitPlayer, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

		void MoveBoxes(AMPPlayer* HitPlayer, const FFramePackage& FrameToCheck);

		void ResetBoxes(AMPPlayer* HitPlayer, const FFramePackage& OrignalFrame);

		void CacheFrame(AMPPlayer* HitPlayer, FFramePackage& CurrentFrame);

		UFUNCTION()
		void ServerScoreRequest
		(
			AMPPlayer* HitPlayer,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime,
			class AWeapons* DamageCauser
		);

public:	

		
};
