
#include "MPPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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


}

void AMPPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMPPlayer::BeginPlay()
{
	Super::BeginPlay();

}


void AMPPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPPlayer::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AMPPlayer::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AMPPlayer::LookRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

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



