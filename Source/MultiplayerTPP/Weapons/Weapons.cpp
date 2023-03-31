// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include"Components/SphereComponent.h"

// Sets default values
AWeapons::AWeapons()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Mesh->SetupAttachment(RootComponent);
	SetRootComponent(Mesh);

	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OverlapAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapAreaSphere"));
	OverlapAreaSphere->SetupAttachment(RootComponent);

	OverlapAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);





}

// Called when the game starts or when spawned
void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OverlapAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OverlapAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	
}

// Called every frame
void AWeapons::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

