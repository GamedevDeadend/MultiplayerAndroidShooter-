// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"
#include "TeamDeathMatch_GS.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API ATeamDeathMatch_GS : public ADeathMatch_GS
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	UPROPERTY()
		TArray<class AMPPlayerState* > RedTeamPlayers;

	UPROPERTY()
		TArray<class AMPPlayerState* > BlueTeamPlayers;

private :

	UPROPERTY(EditAnywhere)
		UMaterial* RedTeamMaterial = nullptr;

	UPROPERTY(EditAnywhere)
		UMaterialInstance* RedTeamMaterialDissolve = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
		float RedTeamScore = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
		float BlueTeamScore = 0.0f;

	UFUNCTION()
		void OnRep_RedTeamScore();

	UFUNCTION()
		void OnRep_BlueTeamScore();
public : 

	FORCEINLINE UMaterialInstance* GetRedTeamMaterialDissolve()const { return RedTeamMaterialDissolve; };
	FORCEINLINE UMaterial* GetRedTeamMaterial()const { return RedTeamMaterial; };
	void SetRedTeamScore();
	void SetBlueTeamScore();

	
};
