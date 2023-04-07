// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapons;
class AMPPlayer;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

	AMPPlayer* Player;

	UPROPERTY(Replicated)
	AWeapons* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAim;

	void SetAiming(bool bIsAiming);

	//Server RPC invoke from clients and execute on server
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

protected:
	
	virtual void BeginPlay() override;


//Getters And Setters
public:	
	

		
};
