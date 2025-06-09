

#include "MPPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include"Net/UnrealNetwork.h"
#include"MultiplayerTPP/Weapons/Weapons.h"
#include"MultiplayerTPP/PlayerComponents/CombatComponent.h"
#include"Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "MultiplayerTPP/MultiplayerTPP.h"
#include "MultiplayerTPP/Controllers/MPPlayerController.h"
#include "MultiplayerTPP/GameMode/DeathMatch_GM.h"
#include "MultiplayerTPP/GameStates/DeathMatch_GS.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"
#include "MultiplayerTPP/Types/WeaponType.h"
#include "Components/BoxComponent.h"
#include "MultiplayerTPP/PlayerComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "MultiplayerTPP/GameStates/TeamDeathMatch_GS.h"
#include "NiagaraFunctionLibrary.h"




AMPPlayer::AMPPlayer()
{

	PrimaryActorTick.bCanEverTick = true;


	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHead = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHead Widget"));
	OverHead->SetupAttachment(GetMesh());

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	CombatComponent->SetIsReplicated(true);

	HeadLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Head Light"));
	HeadLight->SetupAttachment(GetCapsuleComponent());

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline Component "));

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("Lag Compensation Component"));



	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	TurningInplace = ETurningInPlace::ETIP_NotTurning;


	/**
	* Hit boxes for server-side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));

	HitCollisionBoxesMap.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));

	HitCollisionBoxesMap.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));

	HitCollisionBoxesMap.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));

	HitCollisionBoxesMap.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));

	HitCollisionBoxesMap.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));

	HitCollisionBoxesMap.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));

	HitCollisionBoxesMap.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));

	HitCollisionBoxesMap.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));

	HitCollisionBoxesMap.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));

	HitCollisionBoxesMap.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));

	HitCollisionBoxesMap.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));

	HitCollisionBoxesMap.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));

	HitCollisionBoxesMap.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));

	HitCollisionBoxesMap.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));

	HitCollisionBoxesMap.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));

	HitCollisionBoxesMap.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));

	HitCollisionBoxesMap.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));

	HitCollisionBoxesMap.Add(FName("foot_r"), foot_r);

	for (auto& BoxPair : HitCollisionBoxesMap)
	{
		if (BoxPair.Value != nullptr)
		{
			BoxPair.Value->SetCollisionObjectType(ECC_HitBox);
			BoxPair.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

}

void AMPPlayer::PostInitializeComponents()
{

	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->MPPlayer = this;
	}

	if (LeadGainParticleComponent != nullptr)
	{
		LeadGainParticleComponent->DestroyComponent();
	}

	if (LagCompensationComponent != nullptr)
	{
		LagCompensationComponent->Character = this;
		
		if (Controller != nullptr)
		{
			LagCompensationComponent->CharacterController = Cast<AMPPlayerController>(Controller);
		}
	}
}

//This function is used to register replicated variable
void AMPPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This will set Overlappedweapon for replication but intial value will be null
	DOREPLIFETIME_CONDITION(AMPPlayer, OverlappedWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMPPlayer, StartAimRotation);
	DOREPLIFETIME(AMPPlayer, CurrentHealth);
	DOREPLIFETIME(AMPPlayer, bIsGameplayDisabled);
}

void AMPPlayer::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	MPPlayerController = Cast < AMPPlayerController>(Controller);
	if (MPPlayerController)
	{
		MPPlayerController->SetHUDHealth(MaxHealth, CurrentHealth);
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMPPlayer::TakeDamage);

	}
}

void AMPPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float PreviousInterval = PrimaryActorTick.TickInterval;

	if (ENetRole::ROLE_SimulatedProxy && !IsLocallyControlled())
	{

		PrimaryActorTick.TickInterval = 10.0f;
	}
	else
	{
		PrimaryActorTick.TickInterval = PreviousInterval;
	}

	AimOffset(DeltaTime);
	HidePlayerIfCameraTooClose();
	PollInit();

	//if( HasAuthority() && !IsLocallyControlled() )
	//UE_LOG(LogTemp, Warning, TEXT("Pitch Rotation %f"), AO_Pitch);

	//UE_LOG(LogTemp, Error, TEXT("Walk Speed %f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AMPPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPPlayer::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AMPPlayer::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AMPPlayer::LookRight);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMPPlayer::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMPPlayer::EquipWeapon);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, & AMPPlayer::DropWeapon);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMPPlayer::CrouchAction);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMPPlayer::AimPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMPPlayer::AimReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMPPlayer::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMPPlayer::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AMPPlayer::ReloadWeapon);

}

void AMPPlayer::FireButtonPressed()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent)
	{
		CombatComponent->FirePressed(true);
	}
}

void AMPPlayer::FireButtonReleased()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent)
	{
		CombatComponent->FirePressed(false);
	}

}

void AMPPlayer::MoveForward(float Value)
{
	if (bIsGameplayDisabled) { return; }

	if (Controller != nullptr && Value != 0.0f)
	{
		//Instead of characterMesh Rotation we are getting controller rotation
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.0f);

		//Mathematics to get forward vector of controller
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMPPlayer::MoveRight(float Value)
{
	if (bIsGameplayDisabled) { return; }

	if (Controller != nullptr && Value != 0)
	{
		//Instead of characterMesh Rotation we are getting controller rotation
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.0f);

		//Mathematics to get forward vector of controller
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMPPlayer::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMPPlayer::LookRight(float Value)
{
	AddControllerYawInput(Value);
}

void AMPPlayer::EquipWeapon()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent)
	{
		if (HasAuthority())
			CombatComponent->EquipWeapon(OverlappedWeapon);
		else
			ServerEquipPressed();
	}
}

void AMPPlayer::ReloadWeapon()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent != nullptr)
	{
		CombatComponent->Reload();
	}
}

void AMPPlayer::DropWeapon()
{
	//TODO: Create Drop Weapon Functionality
}

void AMPPlayer::ServerEquipPressed_Implementation()
{
	if (CombatComponent)
		CombatComponent->EquipWeapon(OverlappedWeapon);
}

void AMPPlayer::Jump()
{
	if (bIsGameplayDisabled) { return; }
	if (bIsCrouched)
	{
		UnCrouch();
	}

	else
	{
		Super::Jump();
	}
}

void AMPPlayer::CrouchAction()
{
	if (bIsGameplayDisabled) { return; }

	if (bIsCrouched)
		UnCrouch();

	else
		Crouch();
}

void AMPPlayer::AimPressed()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent)
		CombatComponent->SetAiming(true);
}

void AMPPlayer::AimReleased()
{
	if (bIsGameplayDisabled) { return; }

	if (CombatComponent)
		CombatComponent->SetAiming(false);
}

bool AMPPlayer::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool AMPPlayer::IsAiming()
{
	return CombatComponent->bAim;
}

AWeapons* AMPPlayer::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;

	return CombatComponent->EquippedWeapon;
}

FVector AMPPlayer::GetHitTarget() const
{
	return CombatComponent == nullptr ? FVector() : CombatComponent->HitTarget;
}

ECombatState AMPPlayer::GetCombatState() const
{
	if (CombatComponent != nullptr)
	{
		return CombatComponent->CombatState;
	}
	else
	{
		return ECombatState::ECS_MAX;
	}
}


FString AMPPlayer::GetPlayerName()
{
	APlayerState* OurPlayerState = this->GetPlayerState();
	FString OurPlayerName;

	if (OurPlayerState)
		OurPlayerName = OurPlayerState->GetPlayerName();

	return OurPlayerName;
}

//THIS FUNC IS SETTING OVERLAPPEDWEAPON ON EVERY CALL WHICH IN TURN IS CALLING REP NOTIFY
void AMPPlayer::SetOverlappingWeapon(AWeapons* Weapon)
{
	//THIS CONDITION IS TO HIDE PICKUP WIDGET ON SERVER SIDE PAWN

	if (!Weapon)
	{
		OverlappedWeapon->ShowPickupWidget(false);
	}

	OverlappedWeapon = Weapon;

	//THIS CONDITION IS TO SHOW PICKUP WIDGET ON SERVER SIDE PAWN
	if (IsLocallyControlled())
	{
		if (OverlappedWeapon)
		{
			OverlappedWeapon->ShowPickupWidget(true);
		}
	}
}

void AMPPlayer::OnRep_OverlappedWeapon(AWeapons* LastWeapon)
{
	if (OverlappedWeapon)
		OverlappedWeapon->ShowPickupWidget(true);

	if (LastWeapon)
		LastWeapon->ShowPickupWidget(false);
}

void AMPPlayer::OnRep_HealthChange()
{
	PlayHitReactMontage();
	UpdateHealthHUD();
}

void AMPPlayer::PlayFireMontage(bool bAiming)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
	UAnimInstance* PlayerAnimInstance = GetMesh()->GetAnimInstance();
	if (PlayerAnimInstance && FireMontage)
	{
		PlayerAnimInstance->Montage_Play(FireMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		PlayerAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMPPlayer::PlayReloadMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;

	UAnimInstance* PlayerAnimInstance = GetMesh()->GetAnimInstance();
	if (PlayerAnimInstance && ReloadMontage)
	{
		PlayerAnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AR_Auto: 
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_AR_Burst:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_AR_Single:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_ROCKET:
			SectionName = FName("Rifle");

		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");

		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");

			break;
		}

		PlayerAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMPPlayer::PlayHitReactMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) return;
	UAnimInstance* PlayerAnimInstance = GetMesh()->GetAnimInstance();
	if (PlayerAnimInstance && HitReactMontage)
	{
		PlayerAnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("From Front");
		PlayerAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMPPlayer::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (EliminationMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void AMPPlayer::HidePlayerIfCameraTooClose()
{
	if (!IsLocallyControlled()) return;

	if ( (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < AMPPlayer::CameraThreshold)
	{
		GetMesh()->SetVisibility(false);

		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}

	else
	{
		GetMesh()->SetVisibility(true);

		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}

	}

}

void AMPPlayer::UpdateHealthHUD()
{
	MPPlayerController = MPPlayerController == nullptr ? Cast < AMPPlayerController>(Controller) : MPPlayerController;
	if (MPPlayerController)
	{
		MPPlayerController->SetHUDHealth(MaxHealth, CurrentHealth);
	}
}

void AMPPlayer::Elim(bool bIsLeaving)
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}

	MulticastElim(bIsLeaving);
}


void AMPPlayer::MulticastElim_Implementation(bool bIsLeaving)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("On MulticastElim"));

	this->bIsPlayerLeaving = bIsLeaving;
	if (MPPlayerController != nullptr)
	{
		MPPlayerController->SetHUDAmmoCount(0);
	}

	if (DissolveMaterialInstance == nullptr) return;

	bIsEliminated = true;
	PlayElimMontage();

	//Dissolve Material Setup
	DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
	GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
	DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), -0.55f);
	DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.0f);

	//Dissolve TimeLine Function
	StartDissolveMaterial();

	/*Elimination hoverring Bot*/
	if (ElimBotParticleEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotParticleEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	if (LeadGainParticleComponent != nullptr)
	{
		LeadGainParticleComponent->DestroyComponent();
	}


	//Disable Movement
	GetCharacterMovement()->DisableMovement();//Disable Key Input
	GetCharacterMovement()->StopMovementImmediately();//Disable Mouse Inputs Too
	bIsGameplayDisabled = true;

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(ElimDelayTimer, this, &ThisClass::EliminationFinished, ElimDelay);
}

void AMPPlayer::EliminationFinished()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT(" ElimPlayer Finished"));

	ADeathMatch_GM* DeathMatchGM = GetWorld()->GetAuthGameMode<ADeathMatch_GM>();

	if (DeathMatchGM && bIsPlayerLeaving == false)
	{
		DeathMatchGM->RequestRespawn(this, Controller);
	}

	if (IsLocallyControlled() == true && bIsPlayerLeaving == true)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Fired OnLeaving Match Delegate"));
		OnLeavingMatch.Broadcast();
	}
}

void AMPPlayer::StartDissolveMaterial()
{
	// Setting and Binding Runtime Material & Timeline
	if (DissolveCurve == nullptr || DissolveTimeline == nullptr) return;

	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
	DissolveTimeline->Play();
}

void AMPPlayer::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AMPPlayer::PollInit()
{
	if (MPPlayerState == nullptr)
	{
		MPPlayerState = GetPlayerState<AMPPlayerState>();
		if (MPPlayerState)
		{
			MPPlayerState->AddToScore(0.f);
			MPPlayerState->AddToDefeat(0);
		}

		if (LeadGainParticleComponent == nullptr && HasAuthority())
		{
			ADeathMatch_GS* GameState = Cast<ADeathMatch_GS>(UGameplayStatics::GetGameState(this));

			if (GameState != nullptr && GameState->TopScoringPlayers.Contains(MPPlayerState) == true && HasAuthority() == true)
			{
				Mulitcast_GainLead();
			}
		}

		SetMaterialOnRespawn();
	}


}

/// <summary>
/// Function for setting material for TDM according to team selected
/// </summary>
void AMPPlayer::SetMaterialOnRespawn()
{
	if (MPPlayerState != nullptr && MPPlayerState->GetPlayerTeam() == EPlayerTeam::EPT_RED)
	{
		ATeamDeathMatch_GS* TDM_GS = Cast<ATeamDeathMatch_GS>( UGameplayStatics::GetGameState(this) );

		if (TDM_GS != nullptr)
		{
			if (GetMesh() != nullptr)
			{
				GetMesh()->SetMaterial(0, TDM_GS->GetRedTeamMaterial());
				DissolveMaterialInstance = TDM_GS->GetRedTeamMaterialDissolve();
			}
		}
	}
}

void AMPPlayer::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	
	ADeathMatch_GM* DeathMatch_GM = GetWorld()->GetAuthGameMode<ADeathMatch_GM>();

	if (DeathMatch_GM)
	{
		MPPlayerController = MPPlayerController ? MPPlayerController : Cast<AMPPlayerController>(Controller);
		AMPPlayerController* AttackPlayercontroller = Cast<AMPPlayerController>(InstigatorController);

		AMPPlayerState* VictimState = MPPlayerController->GetPlayerState<AMPPlayerState>();
		AMPPlayerState* AttackerState = AttackPlayercontroller->GetPlayerState<AMPPlayerState>();

		if (DeathMatch_GM->CheckIsFriendlyFire(AttackerState, VictimState) == true)
		{
			return;
		}

		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

		PlayHitReactMontage();
		UpdateHealthHUD();

		if (CurrentHealth == 0.0f)
		{
			DeathMatch_GM->PlayerEliminated(this, MPPlayerController, AttackPlayercontroller);
		}
	};


}

void AMPPlayer::ServerLeaveGame_Implementation()
{
	UWorld* World = GetWorld();

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Called ServerLeaveGame"));

	if (World != nullptr)
	{
		ADeathMatch_GM* GameMode = World->GetAuthGameMode<ADeathMatch_GM>();
		MPPlayerState = MPPlayerState == nullptr ? GetPlayerState<AMPPlayerState>() : MPPlayerState;

		if (GameMode != nullptr)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Called KickPlayer"));
			GameMode->KickPlayer(MPPlayerState);
		}
	}
}

void AMPPlayer::Destroyed()
{
	Super::Destroyed();

	if (ElimBotParticleComponent)
	{
		ElimBotParticleComponent->DestroyComponent();
	}

	//if (CombatComponent != nullptr && CombatComponent->EquippedWeapon != nullptr)
	//{
	//	CombatComponent->EquippedWeapon->Destroy();
	//}
}

void AMPPlayer::AimOffset(float DeltaTime)
{
	if (bIsGameplayDisabled)
	{ 
		bUseControllerRotationYaw = false;
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // Yaw aim offset will work only if we are moving 
	{
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		//AO_Yaw = FMath::Clamp(AO_Yaw, -90.0f, 90.0f);

		InterpAo_Yaw = TurningInplace == ETurningInPlace::ETIP_NotTurning ? AO_Yaw : InterpAo_Yaw;

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.0f || bIsInAir)
	{
		StartAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		FVector2D InRange(270.0f, 360.0f);
		FVector2D OutRange(-90.0f, 0.0f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);

	}
}

void AMPPlayer::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp, Error, TEXT("Yaw Rotation Static %f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInplace = ETurningInPlace::ETIP_Right;
	}

	else if (AO_Yaw < -90.f)
	{
		TurningInplace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInplace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAo_Yaw = FMath::FInterpTo(InterpAo_Yaw, 0.0f, DeltaTime, 4.0f);
		AO_Yaw = InterpAo_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInplace = ETurningInPlace::ETIP_NotTurning;
			StartAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void AMPPlayer::Mulitcast_GainLead_Implementation()
{
	if (LeadGainParticleSystem != nullptr)
	{
		if (LeadGainParticleComponent == nullptr)
		{
			LeadGainParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached
			(
				LeadGainParticleSystem,
				GetCapsuleComponent(),
				FName(),
				GetActorLocation() + LeadGainOffset,
				GetActorRotation(),
				EAttachLocation::KeepWorldPosition,
				false
			);
		}
	}

	if (LeadGainParticleComponent != nullptr)
	{
		LeadGainParticleComponent->Activate();
	}
}

void AMPPlayer::Multicast_LossLead_Implementation()
{
	if (LeadGainParticleComponent != nullptr)
	{
		LeadGainParticleComponent->DestroyComponent();
	}
}



