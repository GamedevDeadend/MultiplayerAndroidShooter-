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

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* OverlapAreaSphere;

	UPROPERTY(VisibleAnywhere)
		EWeaponState WeaponState;

protected:
	virtual void BeginPlay() override;

};
