// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// 忽略相机碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 仅复制到拥有 Character 的 Client, 服务器无法触发
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ------------Set Player Input Begin------------------
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Bind actions
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ThisClass::EquipWeapon);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector DirectionF(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); 
		AddMovementInput(DirectionF, MovementVector.Y);
		const FVector DirectionR(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)); 
		AddMovementInput(DirectionR, MovementVector.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookRotation = Value.Get<FVector2D>();
	AddControllerYawInput(LookRotation.X);
	AddControllerPitchInput(LookRotation.Y);
}

void ABlasterCharacter::EquipWeapon(const FInputActionValue& Value)
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);	
		}
		else
		{
			ServerEquipButtonPress();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();	
	}
}

void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased(const FInputActionValue& Value)
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::Jump()
{
	Super::Jump();
}
// ------------Set Player Input End------------------

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	// 如果控制 Character 的是服务器
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPress_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}
