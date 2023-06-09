// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerTPP/Types/TurningInPlace.h"
#include "MultiplayerTPP/Interfaces/InteractWithCrosshairs.h"
#include "MPPlayer.generated.h"

class UCombatComponent;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class USpotLightComponent;

UCLASS()
class MULTIPLAYERTPP_API AMPPlayer : public ACharacter, public IInteractWithCrosshairs
{
	GENERATED_BODY()

public:

	AMPPlayer();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();


protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:

	virtual void PostInitializeComponents() override;
	void Jump()override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void EquipWeapon();
	void CrouchAction();
	void AimPressed();
	void AimReleased();
	void AimOffset(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void HidePlayerIfCameraTooClose();
	void UpdateHealthHUD();

	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// Server side rpc method are already implemneted we have to just extend implementation by using _Implementation 
	UFUNCTION(Server, Reliable)
		void ServerEquipPressed();

	UFUNCTION()
		void OnRep_OverlappedWeapon(class AWeapons* LastWeapon);

	UFUNCTION()
		void OnRep_HealthChange();

	UPROPERTY(EditAnywhere, Category  = "Player Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_HealthChange,VisibleAnywhere, Category = "Player Stats")
	float CurrentHealth = MaxHealth;

	UPROPERTY(EditAnywhere, Category = Combat)
		UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Light)
		USpotLightComponent* HeadLight;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, Category = Camera)
	float CameraThreshold = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadonly, meta = (Allowprivateaccess = true))
		UWidgetComponent* OverHead;

	//Meta specifer to connect variabler to onrep function. ONrep-- will be called as soon as this variable is replicated.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappedWeapon)
		AWeapons* OverlappedWeapon;

	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage*	HitReactMontage;

	float AO_Yaw;
	float AO_Pitch;
	float InterpAo_Yaw;
	ETurningInPlace TurningInplace;
	class AMPPlayerController* MPPlayerController;

	UPROPERTY(Replicated)
		FRotator StartAimRotation;

	//Meta specifier indicating that this variable needs to be replicated


//Getters And Setters
public:

	UFUNCTION(BlueprintCallable)
		FString GetPlayerName();

	void SetOverlappingWeapon(AWeapons* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE void SetBaseAimRotation(FRotator AimRotation) { StartAimRotation = AimRotation; }

	FORCEINLINE float GetAimYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAimPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningState() const { return TurningInplace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	AWeapons* GetEquippedWeapon();
	FVector GetHitTarget()const;
};