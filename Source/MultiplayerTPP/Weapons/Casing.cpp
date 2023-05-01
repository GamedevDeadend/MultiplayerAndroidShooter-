// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	ImpulseScale = FMath::RandRange(2.0f, 6.0f);
	float RandInt = FMath::RandRange(0.0f, 180.f);
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	CasingMesh->AddRelativeRotation(FRotator(0.0f, 0.0f, RandInt));
	CasingMesh->AddImpulse(GetActorForwardVector() * ImpulseScale);
	SetLifeSpan(1.0f);

}

void ACasing::OnHit
(
	UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (ShellHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellHitSound, GetActorLocation());
	}

	CasingMesh->SetSimulatePhysics(false);
	CasingMesh->SetNotifyRigidBodyCollision(false);
	
}

// Called every frame
void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

