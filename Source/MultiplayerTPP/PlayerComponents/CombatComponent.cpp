// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "Engine/SkeletalMeshSocket.h"



UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}



void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EquipWeapon(AWeapons* WeaponToEquip)
{
	if (!Player || !WeaponToEquip)
		return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Player->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Player->GetMesh());
	}
	EquippedWeapon->SetOwner(Player);
	EquippedWeapon->ShowPickupWidget(false);
}

