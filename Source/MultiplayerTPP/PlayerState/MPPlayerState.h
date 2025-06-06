// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MultiplayerTPP/Types/PlayerTeam.h"
#include "MPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPP_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	virtual void AddToScore(float Amt);
	virtual void AddToDefeat(int32 DefeatAmt);
	void DisplayLossingMessage();
	inline void SetCanReplicateDefeat(bool b) { CanReplicateDefeat = b; };

	virtual void PostInitializeComponents()override;
	//virtual void DisplayLosingMessage();

private:

	UPROPERTY(Replicated)
	FString PlayerEosId = "";

	UPROPERTY()
	class ATeamDeathMatch_GS* GameState = nullptr;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	void UpdateHUDScore();
	void UpdateHUDDefeats();
	void SetTeamRelevantMaterial(EPlayerTeam TeamToSet);

	UFUNCTION(NetMulticast, Reliable)
		void OnRep_Team();

	bool CanReplicateDefeat = false;


	UFUNCTION()
		void HideLossingMessage();

	UPROPERTY()
		class AMPPlayer* Character = nullptr;

	UPROPERTY()
		class AMPPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 DefeatsAmt;

	UPROPERTY(EditAnywhere, Category = "Defeated Message")
		FString DisplayMessage = "You were Killed";

	UPROPERTY(ReplicatedUsing  = OnRep_Team)
		EPlayerTeam Team = EPlayerTeam::EPT_NONE;

public:

	FORCEINLINE EPlayerTeam GetPlayerTeam()const { return Team; };
	FORCEINLINE FString GetPlayerId()const { return PlayerEosId; };
	void SetPlayerTeam(EPlayerTeam TeamToSet);


};
 