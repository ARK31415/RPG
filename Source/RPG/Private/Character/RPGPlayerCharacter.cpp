// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RPGPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RPGGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "Component/Input/RPGEnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARPGPlayerCharacter::ARPGPlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	CameraBoom->bUsePawnControlRotation = true; //使用人物控制

	//摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	// GetCharacterMovement()->MaxWalkSpeed = 400.f;
	// GetCharacterMovement()->BrakingDecelerationWalking = 200.f;
}

void ARPGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(InputConfigDataAsset, TEXT("InputConfig is null"))

	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	checkf(InputSubsystem, TEXT("InputSusystem is null"))
	InputSubsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);

	URPGEnhancedInputComponent* RPGInputComponent = CastChecked<URPGEnhancedInputComponent>(PlayerInputComponent);
	RPGInputComponent->BindNativeInputAction(InputConfigDataAsset, RPGGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this,
	                                         &ThisClass::Input_Move);
	RPGInputComponent->BindNativeInputAction(InputConfigDataAsset, RPGGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this,
	                                         &ThisClass::Input_Look);

	// RPGInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed,
	// 											  &ThisClass::Input_AbilityInputReleased);
}

void ARPGPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2d MovementVector = InputActionValue.Get<FVector2d>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARPGPlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2d LookAxisVector = InputActionValue.Get<FVector2d>();

	if (LookAxisVector.X != 0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if (LookAxisVector.Y != 0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
