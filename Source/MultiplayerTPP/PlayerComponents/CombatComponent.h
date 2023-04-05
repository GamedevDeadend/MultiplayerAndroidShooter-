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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void EquipWeapon(AWeapons* Equipweapon);

	friend class AMPPlayer;


private:

	AMPPlayer* Player;
	AWeapons* EquippedWeapon;


protected:
	
	virtual void BeginPlay() override;


//Getters And Setters
public:	
	

		
};
