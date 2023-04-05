

#include "MPPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"Components/WidgetComponent.h"
#include "GameFramework/PlayerState.h"
#include"Net/UnrealNetwork.h"
#include"MultiplayerTPP/Weapons/Weapons.h"
#include"MultiplayerTPP/PlayerComponents/CombatComponent.h"

AMPPlayer::AMPPlayer()
{

	PrimaryActorTick.bCanEverTick = false;

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
}

void AMPPlayer:: PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Player = this;
	}
}
//This function is used to register replicated variable
void AMPPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This will set Overlappedweapon for replication but intial value will be null
	DOREPLIFETIME_CONDITION(AMPPlayer, OverlappedWeapon, COND_OwnerOnly); 
}

void AMPPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AMPPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	if (OverlappedWeapon)
	{
		OverlappedWeapon->ShowPickupWidget(true);
	}
}


void AMPPlayer::OnRep_OverlappedWeapon(AWeapons* LastWeapon)
{
	if (OverlappedWeapon)
		OverlappedWeapon->ShowPickupWidget(true);

	if (LastWeapon)
		LastWeapon->ShowPickupWidget(false);
}

void AMPPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPPlayer::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AMPPlayer::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AMPPlayer::LookRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMPPlayer::EquipWeapon);
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
	if (HasAuthority() || CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappedWeapon);
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



