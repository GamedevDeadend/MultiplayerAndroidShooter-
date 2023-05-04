// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimAsset;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* OverlapAreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickUpWidget;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ACasing> CasingClass;

	UFUNCTION()
		void OnRep_WeaponState();

	//Zooming Attributes

	UPROPERTY(EditAnywhere, Category = "Zoom")
		float ZoomFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
		float ZoomInterpSpeed = 20.0f;


protected:

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
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return Mesh; }
	FORCEINLINE float GetZoomedFOV()const { return ZoomFOV; }
	FORCEINLINE float GetZoomInterpSpeed()const { return ZoomInterpSpeed; }
};
