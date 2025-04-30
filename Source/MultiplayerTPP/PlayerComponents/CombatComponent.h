// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerTPP/WidgetsHud/Hud/MPPlayerHUD.h"
#include "MultiplayerTPP/Types/WeaponType.h"
#include "MultiplayerTPP/Types/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERTPP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCombatComponent();

	friend class AMPPlayer;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapons* Equipweapon);

	void UpdateEquippedWeaponAmmo();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:

	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	//Server RPC invoke from clients and execute on server
	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
		void OnRep_WeaponEquip();

	void FirePressed(bool bPressed);

	void Fire();

	void BurstFire();

	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
		void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void HandleReload();

	void SetHUD(float DeltaTime);

private:


	class AMPPlayer* MPPlayer;
	class AMPPlayerController* MPPlayerController;
	class AMPPlayerHUD* HUD;

	bool bFireButtonPressed;
	bool bIsWeaponEmpty;
	FVector_NetQuantize HitTarget;

	float VelocityFactor;
	float JumpFactor;
	float AimFactor;
	float ShootingFactor;
	int BurstFireCount = 0;
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle BurstFireTimerHandle;


	FHUDPackage MPPlayerHUDPackage;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float BaseJumpVelocity;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float EquipJumpVelociy;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float BaseWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Combat Movement", meta = (Allowprivateaccess = true))
		float AimWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Combat Crosshairs Color", meta = (Allowprivateaccess = true))
		FLinearColor EnemyAimColor;

	UPROPERTY(EditAnywhere, Category = "Combat Crosshairs Color", meta = (Allowprivateaccess = true))
		FLinearColor NormalAimColor;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquip)
		AWeapons* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAim;

	//Zooming Parameters

	float DefaultFOV, CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Zoom", meta = (Allowprivateaccess = true))
		float ZoomFOV;

	UPROPERTY(EditAnywhere, Category = "Zoom", meta = (Allowprivateaccess = true))
		float ZoomOutInterp = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (AllowPrivateaccess = true))
		int32 DeafaultAvailableAmmo = 30;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeaponAmmo)
		int32 EquippedWeaponAmmo;

	TMap<EWeaponType, int32> AmmunationMap;

	void InterpFOV(float DeltaTime);
	void InitPlayerAmmunationMap();
	void Reload();

	UFUNCTION(Server, Reliable)
		void ServerReload();

	UFUNCTION()
		void OnRep_EquippedWeaponAmmo();

	UFUNCTION()
		void OnRep_CombatState();


	//Getters And Setters
public:
};
