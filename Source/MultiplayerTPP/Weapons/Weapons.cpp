// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include"Components/SphereComponent.h"
#include"Components/WidgetComponent.h"
#include"MultiplayerTPP/Character/MPPlayer.h"
#include"Net/UnrealNetwork.h"


// Sets default values
AWeapons::AWeapons()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

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


}


void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}


	//We are binding delegates on server side only 
	if (HasAuthority())
	{
		OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OverlapAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		OverlapAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapons::OnSphereOverlap);
		OverlapAreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapons::OnSphereEndOverlap);
	}
	
}

void AWeapons::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AWeapons::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapons, WeaponState);
}


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
		Player->SetOverlappingWeapon(this);
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
		Player->SetOverlappingWeapon(nullptr);
}

void AWeapons::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AWeapons::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			//OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AWeapons::ShowPickupWidget(bool bShowWidget)
{
	if (PickUpWidget)
		PickUpWidget->SetVisibility(bShowWidget);
}

