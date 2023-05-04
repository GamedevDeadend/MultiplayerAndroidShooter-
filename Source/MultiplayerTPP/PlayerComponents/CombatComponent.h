// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerTPP/WidgetsHud/Hud/MPPlayerHUD.h"
#include "CombatComponent.generated.h"

class AWeapons;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTPP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCombatComponent();

	friend class AMPPlayer;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapons* Equipweapon);



private:


	class AMPPlayer* Player;
	class AMPPlayerController* PlayerController;
	class AMPPlayerHUD* HUD;
	bool bFireButtonPressed;
	FVector_NetQuantize HitTarget;
	float VelocityFactor;
	float JumpFactor;
	float AimFactor;
	float ShootingFactor;
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float BaseJumpVelocity;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float EquipJumpVelociy;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float BaseWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float AimWalkSpeed;

	//Zooming Parameters

	float DefaultFOV, CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Zoom", meta = (Allowprivateaccess = true))
		float ZoomFOV;

	UPROPERTY(EditAnywhere, Category = "Zoom", meta = (Allowprivateaccess = true))
		float ZoomOutInterp = 30.0f;


	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquip)
		AWeapons* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAim;

	void SetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_WeaponEquip();

	//Server RPC invoke from clients and execute on server

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
		void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);

	void FirePressed(bool bPressed);
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUD(float DeltaTime);
	void InterpFOV(float DeltaTime);

protected:

	virtual void BeginPlay() override;


	//Getters And Setters
public:
};
