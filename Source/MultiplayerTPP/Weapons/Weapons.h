// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayerTPP/Types/WeaponType.h"
#include "Weapons.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Intial UMETA(DisplayName = "Intial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),

	EWS_MAX UMETA(DisplayName = "MAX State"),//For checking max no. of states
};

UCLASS()
class MULTIPLAYERTPP_API AWeapons : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeapons();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	virtual void Reload(int32& CarriedAmmo);
	virtual void Dropped();
	void SetHUDAmmo();
	void AmmoSpend();

	UFUNCTION()
	void OnHighPing(bool bIsHighPing);


	//Crosshairs

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
		class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
		UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
		UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
		UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
		UTexture2D* CrosshairsRight;


	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (ClampMin = "-30.0", ClampMax = "30.0"))
		float CrosshairsScale;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class USoundCue* EquipSound;

private:

	UFUNCTION()
		void OnRep_WeaponState();

	UFUNCTION(Client, Reliable)
		void ClientAmmoSpend(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
		void ClientAddAmmoForReload(int32 AmmoAdded);

	// Weapon Type
	UPROPERTY(EditAnywhere, Category = " Weapon Type")
		EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = " Weapon Data Asset")
		class UWeaponDataAsset* WeaponAsset;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimAsset;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* OverlapAreaSphere;


	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickUpWidget = nullptr;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ACasing> CasingClass;

	//Zooming Attributes

	UPROPERTY(EditAnywhere, Category = "Zoom")
		float ZoomFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
		float ZoomInterpSpeed = 20.0f;

	UPROPERTY()
		int32 Ammo;

	/// <summary>
	/// To track no of predection made by client for Ammo Spend ( Implemnting Server reconcialtion for Client Side Prediction) 
	/// </summary>
	UPROPERTY()
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
		int32 MagCapacity = 30;

	UPROPERTY()
		class AMPPlayer* OwnerCharacter;

	UPROPERTY()
		class AMPPlayerController* OwnerController;
			

protected:

	UPROPERTY(Replicated, EditAnywhere)
		bool bIsUsingSSR = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		int32 Damage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		int32 HeadDamage = 40.0f;

	virtual void OnRep_Owner() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
		virtual void OnSphereOverlap
		(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComponent,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION(BlueprintCallable)
		virtual void OnSphereEndOverlap
		(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComponent,
			int32 OtherBodyIndex
		);



	//Getters And Setters
public:

	void SetWeaponState(EWeaponState State);
	void UnbindHighPingDelegate();
	void BindHighPingDelegate();
	//void SetIsUsingSSR(bool bCanUse) { bIsUsingSSR = bCanUse; }
	FORCEINLINE void SetAmmo(int32 UpdatedAmmo) { Ammo = UpdatedAmmo; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return Mesh; }
	FORCEINLINE float GetZoomedFOV()const { return ZoomFOV; }
	FORCEINLINE float GetZoomInterpSpeed()const { return ZoomInterpSpeed; }
	FORCEINLINE bool GetIsEmpty()const { return Ammo <= 0; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity()const { return MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE UWeaponDataAsset* GetWeaponDataAsset()const { return WeaponAsset; }
	FORCEINLINE int32 GetDamage()const { return Damage; }
	FORCEINLINE int32 GetHeadDamage()const { return HeadDamage; }

};
