// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/PlayerComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMPPlayer* MPPlayer = Cast<AMPPlayer>(OtherActor);
	if (MPPlayer)
	{
		UCombatComponent* Combat = MPPlayer->GetCombatComponent();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}