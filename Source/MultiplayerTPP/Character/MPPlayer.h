// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MPPlayer.generated.h"

UCLASS()
class MULTIPLAYERTPP_API AMPPlayer : public ACharacter
{
	GENERATED_BODY()

public:

	AMPPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


private:

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);

	UFUNCTION()
	void OnRep_OverlappedWeapon(class AWeapons* LastWeapon);


	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadonly, meta = (Allowprivateaccess = true))
		class UWidgetComponent* OverHead;

	//meta specifier indicating that this variable needs to be replicated
	//UPROPERTY(Replicated)
	
	//Meta specifer to connect variabler to onrep function. ONrep-- will be called as soon as this variable is replicated.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappedWeapon) 
		AWeapons* OverlappedWeapon;

// Public Section for Simple Getters and Setters
public:

	UFUNCTION(BlueprintCallable)
		FString GetPlayerName();

	void SetOverlappingWeapon(AWeapons* Weapon);

};
