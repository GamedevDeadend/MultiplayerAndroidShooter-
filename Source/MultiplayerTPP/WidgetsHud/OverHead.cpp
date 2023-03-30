// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHead.h"
#include "Components/TextBlock.h"


void UOverHead::SetDisplayText(FString TextValue)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextValue));
	}
}

void UOverHead::ShowPlayerNetRole(APawn* PlayerPawn)
{
	ENetRole PlayerNetRole = PlayerPawn->GetLocalRole();
	FString Role;

	switch (PlayerNetRole)
	{
	case  ENetRole::ROLE_Authority:
		Role = FString(" ServerRole");
		break;

	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;

	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;

	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}

	FString RoleDisplayText = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(RoleDisplayText);
}

void UOverHead::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}