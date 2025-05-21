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
		float Time = 0.0f;

	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfoMap;

	UPROPERTY()
		AMPPlayer* Character = nullptr;
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

	/*
	*	 HitScans Weapon
	*/
	FHitResult_SSR ServerSideRewind
	(
		class AMPPlayer* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	/*
	* Projectile Weapon SSR
	*/FHitResult_SSR ProjectileWeapon_SSR
	(
		class AMPPlayer* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100 InitialVelocity,
		float HitTime
	);


	UFUNCTION(Server, Reliable)
	void ServerScoreRequest
	(
		AMPPlayer* HitPlayer,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		class AWeapons* DamageCauser
	);

	UFUNCTION(Server, Reliable)
	void ServerProjectileWeaponScoreRequest
	(
		AMPPlayer* HitPlayer,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);


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

		void FillFrameToCheck(FFramePackage& Pakckage, AMPPlayer* HitCharacter, float HitTime);

		void SaveFramePackage(FFramePackage& Package);

		void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

		void EnableCharacterMeshCollision(AMPPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

		FFramePackage InterpFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

		/*
		* Confirm Hit SSR Hitscan Weapons
		*/

		FHitResult_SSR ConfirmHit_SSR
		(
			const FFramePackage& FrameToCheck,
			AMPPlayer* HitPlayer,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation
		);

		/*
		* Confirm Hit SSR Projectile Weapons
		*/
		FHitResult_SSR ConfirmHit_Projectile_SSR
		(
			const FFramePackage& FrameToCheck,
			class AMPPlayer* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100 InitialVelocity,
			float HitTime
		);



		void MoveBoxes(AMPPlayer* HitPlayer, const FFramePackage& FrameToCheck);

		void ResetBoxes(AMPPlayer* HitPlayer, const FFramePackage& OrignalFrame);

		void CacheFrame(AMPPlayer* HitPlayer, FFramePackage& CurrentFrame);

public:	

		
};
