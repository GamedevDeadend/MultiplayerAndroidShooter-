// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include"Components/SphereComponent.h"
#include"Components/WidgetComponent.h"
#include"MultiplayerTPP/Character/MPPlayer.h"

// Sets default values
AWeapons::AWeapons()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OverlapAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapAreaSphere"));
	OverlapAreaSphere->SetupAttachment(RootComponent);

	OverlapAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup OverHead"));
	PickUpWidget->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}

	if (HasAuthority())
	{
		OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OverlapAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		OverlapAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapons::OnSphereOverlap);
	}
	
}

// Called every frame
void AWeapons::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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



void AWeapons::ShowPickupWidget(bool bShowWidget)
{
	if (PickUpWidget)
		PickUpWidget->SetVisibility(bShowWidget);
}

