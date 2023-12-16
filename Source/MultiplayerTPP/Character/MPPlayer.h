// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerTPP/Types/TurningInPlace.h"
#include "MultiplayerTPP/Interfaces/InteractWithCrosshairs.h"
#include "Components/TimelineComponent.h"
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
	void PlayElimMontage();
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();

	virtual void Destroyed() override;


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
	void EliminationFinished();
	/// <summary>
	/// poll for any relevant classes and intialize them.
	/// </summary>
	void PollInit();

	UFUNCTION()
		void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// Server side rpc method are already implemneted we have to just extend implementation by using _Implementation 
	UFUNCTION(Server, Reliable)
		void ServerEquipPressed();

	UFUNCTION()
		void OnRep_OverlappedWeapon(class AWeapons* LastWeapon);

	UFUNCTION()
		void OnRep_HealthChange();

	UFUNCTION()
		void StartDissolveMaterial();

	UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);



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

	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* EliminationMontage;

	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.0f;

	//Meta specifier indicating that this variable needs to be replicated
	UPROPERTY(Replicated)
		FRotator StartAimRotation;

	/* Dissolve Material Variables*/

	//Material Instance Set on blueprint, through which it's Dynamic instance will be created
	UPROPERTY(EditAnywhere, Category = Elimination)
		UMaterialInstance* DissolveMaterialInstance;

	//Dynamic instance of our elimination material
	UPROPERTY(VisibleAnywhere, Category = Elimination)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	//Elim_Bot
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotParticleComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotParticleEffect;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;
	
	FOnTimelineFloat DissolveTrack;
	float AO_Yaw;
	float AO_Pitch;
	float InterpAo_Yaw;
	bool bIsEliminated = false;
	ETurningInPlace TurningInplace;
	class AMPPlayerController* MPPlayerController;
	FTimerHandle ElimDelayTimer;
	class AMPPlayerState* MPPlayerState;

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
	FORCEINLINE bool GetIsEliminated()const { return bIsEliminated;  }
	FORCEINLINE float GetHealth()const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	AWeapons* GetEquippedWeapon();
	FVector GetHitTarget()const;
};