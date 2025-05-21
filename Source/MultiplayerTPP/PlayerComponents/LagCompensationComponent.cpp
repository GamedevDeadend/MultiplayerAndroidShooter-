// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "MultiplayerTPP/PlayerComponents/CombatComponent.h"
#include "MultiplayerTPP/MultiplayerTPP.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BuildFrameHistory();
}

void ULagCompensationComponent::BuildFrameHistory()
{
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage Package;
		SaveFramePackage(Package);
		FrameHistory.AddHead(Package);
	}
	else
	{
		float HistoryTime = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

		while (HistoryTime > MaxFrameHistoryTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryTime = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage Package;
		SaveFramePackage(Package);
		FrameHistory.AddHead(Package);

		//ShowFramePackage(Package, FColor::Green);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(AMPPlayer* HitPlayer, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapons* DamageCauser)
{
	FHitResult_SSR Confirm = ServerSideRewind(HitPlayer, TraceStart, HitLocation, HitTime);

	if (Character && HitPlayer && DamageCauser && Confirm.bIsHit)
	{
		UGameplayStatics::ApplyDamage(
			HitPlayer,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ServerProjectileWeaponScoreRequest_Implementation(AMPPlayer* HitPlayer, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On Server and Projectile Score request RPC"));

	FHitResult_SSR Confirm = ProjectileWeapon_SSR(HitPlayer, TraceStart, InitialVelocity, HitTime);


	if (Character != nullptr && HitPlayer != nullptr && Confirm.bIsHit)
	{
		AWeapons* DamageCauser = nullptr;
		if (Character->GetCombatComponent() != nullptr)
		{
			DamageCauser = Character->GetCombatComponent()->GetEquippedWeapon();
		}

		UGameplayStatics::ApplyDamage(
			HitPlayer,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

FHitResult_SSR ULagCompensationComponent::ServerSideRewind(AMPPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr
		|| HitCharacter->GetLagCompensationComponent() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn == true) { return FHitResult_SSR{}; }

	FFramePackage FrameToBeChecked;
	FillFrameToCheck(FrameToBeChecked, HitCharacter, HitTime);



	return ConfirmHit_SSR(FrameToBeChecked, HitCharacter, TraceStart, HitLocation);
}

FHitResult_SSR ULagCompensationComponent::ProjectileWeapon_SSR(AMPPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100 InitialVelocity, float HitTime)
{
	FFramePackage FrameToBeChecked;

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On ProjectileWeapon_SSR"));

	FillFrameToCheck(FrameToBeChecked, HitCharacter, HitTime);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("On ProjectileWeapon_SSR FrameToBeChecked Filled"));

	if (FrameToBeChecked.HitBoxInfoMap.IsEmpty() == true)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("On ProjectileWeapon_SSR FrameToBeChecked EMPTY"));
		return FHitResult_SSR{ false, false };
	}

	return ConfirmHit_Projectile_SSR(FrameToBeChecked, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

void ULagCompensationComponent::FillFrameToCheck(FFramePackage& Package, AMPPlayer* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;

	if (bReturn)
	{
		return;
	}

	bool bShouldInterpolate = true;

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Oldest Time %f  Hit Time %f nw %f"), OldestHistoryTime, HitTime, NewestHistoryTime) );

	if (OldestHistoryTime > HitTime)
	{
		return;
	}

	if (OldestHistoryTime == HitTime)
	{
		Package = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	if (NewestHistoryTime <= HitTime)
	{
		Package = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime?
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;

		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check
	{
		Package = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		Package = InterpFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	Package.Character = HitCharacter;
	return;
}

FFramePackage ULagCompensationComponent::InterpFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Difference = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = (HitTime - OlderFrame.Time) / Difference;

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YougerPair : YoungerFrame.HitBoxInfoMap)
	{
		const FName& BoxName = YougerPair.Key;

		const FBoxInformation& OlderBoxInfo = OlderFrame.HitBoxInfoMap[BoxName];
		const FBoxInformation& YoungerBoxInfo = YougerPair.Value;

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.0f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.0f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;

		InterpFramePackage.HitBoxInfoMap.Add(BoxName, InterpBoxInfo);
	}


	return InterpFramePackage;
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AMPPlayer>(GetOwner()) : Character;
	if (Character != nullptr)
	{
		float Time = GetWorld()->GetTimeSeconds();
		Package.Time = Time;
		for (auto& BoxPair : Character->HitCollisionBoxesMap)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfoMap.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FHitResult_SSR ULagCompensationComponent::ConfirmHit_SSR(const FFramePackage& FrameToCheck, AMPPlayer* HitPlayer, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitPlayer == nullptr)
	{
		return FHitResult_SSR();
	}

	FFramePackage CurrentFrame;

	CacheFrame(HitPlayer, CurrentFrame);
	MoveBoxes(HitPlayer, FrameToCheck);
	EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::NoCollision);

	bool bIsHeadShot = false;

	for (auto& pair : HitPlayer->HitCollisionBoxesMap)
	{
		pair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		pair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	FHitResult HitResult;

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_HitBox);

		if (HitResult.bBlockingHit)
		{
			FString HitBoxName = "";
			UBoxComponent* HitBox = Cast<UBoxComponent>( HitResult.GetComponent());
			HitBox->GetName(HitBoxName);

			FColor Color = FColor::Red;

			if (HitBoxName == "head")
			{
				Color = FColor::Cyan;
				bIsHeadShot = true;
			}

			DrawDebugSphere(GetWorld(), HitResult.Location, 5.0f, 12.0f, Color, false, 4.0f, 0.0f, 2.0f);

			ResetBoxes(HitPlayer, CurrentFrame);
			EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::QueryAndPhysics);

			for (auto& pair : HitPlayer->HitCollisionBoxesMap)
			{
				pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			return FHitResult_SSR{ true, bIsHeadShot };
		}
	}

	ResetBoxes(HitPlayer, CurrentFrame);
	EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::QueryAndPhysics);

	for (auto& pair : HitPlayer->HitCollisionBoxesMap)
	{
		pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return FHitResult_SSR{ false, false };
}

FHitResult_SSR ULagCompensationComponent::ConfirmHit_Projectile_SSR(const FFramePackage& FrameToCheck, AMPPlayer* HitPlayer, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100 InitialVelocity, float HitTime)
{

	if (HitPlayer == nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("HitPlayer Is Null"));
		return FHitResult_SSR();
	}

	FFramePackage CurrentFrame;

	CacheFrame(HitPlayer, CurrentFrame);
	MoveBoxes(HitPlayer, FrameToCheck);
	EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::NoCollision);

	bool bIsHeadShot = false;

	for (auto& pair : HitPlayer->HitCollisionBoxesMap)
	{
		if (pair.Value != nullptr)
		{
			pair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			pair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		}
	}

	FPredictProjectilePathParams PathParams;
	FPredictProjectilePathResult PathResult;

	PathParams.bTraceWithCollision = true;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.MaxSimTime = MaxFrameHistoryTime;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.0f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());

	PathParams.DrawDebugTime = MaxFrameHistoryTime;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Predicting Projectile"));
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Is Blocking Hit True") );
		FString HitBoxName = "";
		UBoxComponent* HitBox = Cast<UBoxComponent>(PathResult.HitResult.GetComponent());
		HitBox->GetName(HitBoxName);

		FColor Color = FColor::Red;

		if (HitBoxName == "head")
		{
			Color = FColor::Cyan;
			bIsHeadShot = true;
		}

		DrawDebugSphere(GetWorld(), PathResult.HitResult.Location, 25.0f, 12, Color, true, 4.0f, 0.0f, 2.0f);

		ResetBoxes(HitPlayer, CurrentFrame);
		EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::QueryAndPhysics);

		for (auto& pair : HitPlayer->HitCollisionBoxesMap)
		{
			pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}


		return FHitResult_SSR{ true, bIsHeadShot };
	}

	ResetBoxes(HitPlayer, CurrentFrame);
	EnableCharacterMeshCollision(HitPlayer, ECollisionEnabled::QueryAndPhysics);

	for (auto& pair : HitPlayer->HitCollisionBoxesMap)
	{
		pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return FHitResult_SSR{ false, false };
}

/// <summary>
/// Move Boxes of Hit Player to the Frame that is needed to checked
/// </summary>
/// <param name="HitPlayer"></param>
/// <param name="FrameToCheck"></param>
void ULagCompensationComponent::MoveBoxes(AMPPlayer* HitPlayer, const FFramePackage& FrameToCheck)
{
	for (auto& HitPair : HitPlayer->HitCollisionBoxesMap)
	{
		if (HitPair.Value != nullptr)
		{
			if (FrameToCheck.HitBoxInfoMap.Contains(HitPair.Key))
			{
				HitPair.Value->SetWorldLocation(FrameToCheck.HitBoxInfoMap[HitPair.Key].Location);
				HitPair.Value->SetWorldRotation(FrameToCheck.HitBoxInfoMap[HitPair.Key].Rotation);
				HitPair.Value->SetBoxExtent(FrameToCheck.HitBoxInfoMap[HitPair.Key].BoxExtent);
			}
		}
	}
}

void ULagCompensationComponent::ResetBoxes(AMPPlayer* HitPlayer, const FFramePackage& OrignalFrame)
{
	for (auto& HitPair : HitPlayer->HitCollisionBoxesMap)
	{
		if (HitPair.Value != nullptr)
		{
			HitPair.Value->SetWorldLocation(OrignalFrame.HitBoxInfoMap[HitPair.Key].Location);
			HitPair.Value->SetWorldRotation(OrignalFrame.HitBoxInfoMap[HitPair.Key].Rotation);
			HitPair.Value->SetBoxExtent(OrignalFrame.HitBoxInfoMap[HitPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::CacheFrame(AMPPlayer* HitPlayer, FFramePackage& CurrentFrame)
{
	if (HitPlayer == nullptr) {return;}

	for (auto& HitPair : HitPlayer->HitCollisionBoxesMap)
	{
		FBoxInformation BoxInfo;
		BoxInfo.Location = HitPair.Value->GetComponentLocation();
		BoxInfo.Rotation = HitPair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = HitPair.Value->GetScaledBoxExtent();
		CurrentFrame.HitBoxInfoMap.Add(HitPair.Key, BoxInfo);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfoMap)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			MaxFrameHistoryTime
		);
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AMPPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}



