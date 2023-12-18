

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
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerTPP/PlayerState/MPPlayerState.h"


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


	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	TurningInplace = ETurningInPlace::ETIP_NotTurning;
}

void AMPPlayer::PostInitializeComponents()
{

	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->MPPlayer = this;
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
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMPPlayer::CrouchAction);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMPPlayer::AimPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMPPlayer::AimReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMPPlayer::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMPPlayer::FireButtonReleased);
}

void AMPPlayer::FireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->FirePressed(true);
	}
}

void AMPPlayer::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FirePressed(false);
	}

}

void AMPPlayer::MoveForward(float Value)
{
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
	if (CombatComponent)
	{
		if (HasAuthority())
			CombatComponent->EquipWeapon(OverlappedWeapon);
		else
			ServerEquipPressed();
	}
}

void AMPPlayer::ServerEquipPressed_Implementation()
{
	if (CombatComponent)
		CombatComponent->EquipWeapon(OverlappedWeapon);
}

void AMPPlayer::Jump()
{
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
	if (bIsCrouched)
		UnCrouch();

	else
		Crouch();
}

void AMPPlayer::AimPressed()
{
	if (CombatComponent)
		CombatComponent->SetAiming(true);
}

void AMPPlayer::AimReleased()
{
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

void AMPPlayer::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}

	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimDelayTimer, this, &ThisClass::EliminationFinished, ElimDelay);
}

void AMPPlayer::MulticastElim_Implementation()
{
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


	//Disable Movement
	GetCharacterMovement()->DisableMovement();//Disable Key Input
	GetCharacterMovement()->StopMovementImmediately();//Disable Mouse Inputs Too
	if (MPPlayerController)
	{
		DisableInput(MPPlayerController);
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMPPlayer::EliminationFinished()
{
	ADeathMatch_GM* DeathMatchGM = GetWorld()->GetAuthGameMode<ADeathMatch_GM>();

	if (DeathMatchGM)
	{
		DeathMatchGM->RequestRespawn(this, Controller);
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
	}
}

void AMPPlayer::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	PlayHitReactMontage();
	UpdateHealthHUD();

	if (CurrentHealth == 0.0f)
	{
		ADeathMatch_GM* DeathMatch_GM = GetWorld()->GetAuthGameMode<ADeathMatch_GM>();

		if (DeathMatch_GM)
		{
			MPPlayerController = MPPlayerController ? MPPlayerController : Cast<AMPPlayerController>(Controller);
			AMPPlayerController* AttackPlayercontroller = Cast<AMPPlayerController>(InstigatorController);

			DeathMatch_GM->PlayerEliminated(this, MPPlayerController, AttackPlayercontroller);
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
}

void AMPPlayer::AimOffset(float DeltaTime)
{

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
	UE_LOG(LogTemp, Error, TEXT("Yaw Rotation Static %f"), AO_Yaw);
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



