// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include"Components/SphereComponent.h"
#include"Components/WidgetComponent.h"
#include"MultiplayerTPP/Character/MPPlayer.h"
#include"Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/Texture2D.h"
#include "Math/UnrealMathUtility.h"
#include "MultiplayerTPP/DataAssets/WeaponDataAsset.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"


// Sets default values
AWeapons::AWeapons()
{
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(true);
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OverlapAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapAreaSphere"));
	OverlapAreaSphere->SetupAttachment(RootComponent);

	OverlapAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OverlapAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);


	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup OverHead"));
	PickUpWidget->SetupAttachment(RootComponent);

	Ammo = MagCapacity;
}



void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}


	//We are binding delegates on server side only 
		OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OverlapAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		OverlapAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapons::OnSphereOverlap);
		OverlapAreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapons::OnSphereEndOverlap);
}

void AWeapons::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AWeapons::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapons, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapons, bIsUsingSSR, COND_OwnerOnly);
}

void AWeapons::Fire(const FVector& HitTarget)
{
	if (FireAnimAsset)
	{
		Mesh->PlayAnimation(FireAnimAsset,false);
	}

	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = Mesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(Mesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);
			}
		}
	}

	AmmoSpend();
}

/// <summary>
/// Reload a Weapon
/// </summary>
/// <param name="CarriedAmmo"></param>
void AWeapons::Reload(int32& CarriedAmmo)
{
		int32 RoomInMag = MagCapacity - Ammo;
		int32 least = FMath::Min(RoomInMag, CarriedAmmo);
		CarriedAmmo = FMath::Max(0, (CarriedAmmo - least));
		Ammo += least;
		SetHUDAmmo();

		if (HasAuthority())
		{
			ClientAddAmmoForReload(least);
		}
}

void AWeapons::ClientAddAmmoForReload_Implementation(int32 AmmoAdded)
{
	if (HasAuthority()) return;

	Ammo += AmmoAdded;
	SetHUDAmmo();
}

void AWeapons::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPPlayer>(GetOwner()) : OwnerCharacter;

	if (OwnerCharacter != nullptr)
	{
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Valid Character"));
		//}

		OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->Controller) : OwnerController;

		if (OwnerController != nullptr)
		{

			OwnerController->SetHUDAmmoCount(Ammo);
		}
	}
}

/// <summary>
/// Function called to update Ammo Count
/// </summary>
void AWeapons::AmmoSpend()
{
	Ammo = FMath::Clamp( Ammo-1 , 0, MagCapacity);
	SetHUDAmmo();

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, FString("Inside Set Ammo Spend"));
	//}


	if (HasAuthority())
	{
		ClientAmmoSpend(Ammo);
	}
	else
	{
		++Sequence;
	}

}

void AWeapons::OnHighPing(bool bIsHighPing)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server SSR IS OFF NOW"));
	bIsUsingSSR = !bIsHighPing;
}

void AWeapons::ClientAmmoSpend_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;

	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

/// <summary>
/// Rep Notifier called when Ammo Count is updated
/// </summary>
//void AWeapons::OnRep_Ammo()
//{
//	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPPlayer>(GetOwner()) : OwnerCharacter;
//	SetHUDAmmo();
//}

void AWeapons::OnSphereOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)

{
	AMPPlayer* Player = Cast<AMPPlayer>(OtherActor);

	if (Player)
	{
		Player->SetOverlappingWeapon(this);
	}
}

void AWeapons::OnSphereEndOverlap
(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex
)
{
	AMPPlayer* Player = Cast<AMPPlayer>(OtherActor);
	if (Player)
	{
		Player->SetOverlappingWeapon(nullptr);
	}
}

void AWeapons::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	Mesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AWeapons::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	//AMPPlayerController* MPPlayerController = Cast<AMPPlayerController>(Cast<AMPPlayer>);

	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
			Mesh->SetSimulatePhysics(false);
			Mesh->SetEnableGravity(false);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ShowPickupWidget(false);
			OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BindHighPingDelegate();
		break; 

		case EWeaponState::EWS_Dropped:
			if (HasAuthority())
			{
				OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(true);
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			UnbindHighPingDelegate();
		break;
	}
}

void AWeapons::UnbindHighPingDelegate()
{
	if (OwnerCharacter != nullptr)
	{

		OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->Controller) : OwnerController;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("UnBinding  Delegate"));


		if (OwnerController != nullptr && HasAuthority() && OwnerController->HighPingDelegate.IsBound() == true&& bIsUsingSSR)
		{

			OwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapons::OnHighPing);
		}
	}
}

void AWeapons::BindHighPingDelegate()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPPlayer>(GetOwner()) : OwnerCharacter;

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Binding  Delegate"));

	if (OwnerCharacter != nullptr)
	{

		OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->Controller) : OwnerController;

		if (OwnerController != nullptr && HasAuthority() && OwnerController->HighPingDelegate.IsBound() == false && bIsUsingSSR )
		{

			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Binded DELEGATE"));
			OwnerController->HighPingDelegate.AddDynamic(this, &AWeapons::OnHighPing);
		}
	}
}

void AWeapons::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
			Mesh->SetSimulatePhysics(false);
			Mesh->SetEnableGravity(false);
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

			//OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		case EWeaponState::EWS_Dropped:
			Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(true);
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	}
}

void AWeapons::ShowPickupWidget(bool bShowWidget)
{
	if (this && PickUpWidget != nullptr)
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

void AWeapons::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}
