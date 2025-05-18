// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "MultiplayerTPP/Character/MPPlayer.h"
#include "MultiplayerTPP/Weapons/Weapons.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/BoxComponent.h"

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

		ShowFramePackage(Package, FColor::Green);
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

FHitResult_SSR ULagCompensationComponent::ServerSideRewind(AMPPlayer* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr
		|| HitCharacter->GetLagCompensationComponent() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn == true) { return FHitResult_SSR{}; }

	FFramePackage FrameToBeChecked;

	bool bShouldInterpolate = true;

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		return FHitResult_SSR{};;
	}

	if (OldestHistoryTime == HitTime)
	{
		FrameToBeChecked = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	if (NewestHistoryTime <= HitTime)
	{
		FrameToBeChecked = History.GetHead()->GetValue();
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
		FrameToBeChecked = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		FrameToBeChecked = InterpFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return ConfirmHit_SSR(FrameToBeChecked, HitCharacter, TraceStart, HitLocation);
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
		pair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}

	FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	FHitResult HitResult;

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

		if (HitResult.bBlockingHit)
		{
			FString HitBoxName = "";
			UBoxComponent* HitBox = Cast<UBoxComponent>( HitResult.GetComponent());
			HitBox->GetName(HitBoxName);

			if (HitBoxName == "head")
			{
				bIsHeadShot = true;
			}

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
			HitPair.Value->SetWorldLocation(FrameToCheck.HitBoxInfoMap[HitPair.Key].Location);
			HitPair.Value->SetWorldRotation(FrameToCheck.HitBoxInfoMap[HitPair.Key].Rotation);
			HitPair.Value->SetBoxExtent(FrameToCheck.HitBoxInfoMap[HitPair.Key].BoxExtent);
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



