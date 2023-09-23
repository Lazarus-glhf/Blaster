// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileBullet::AProjectileBullet()
{
	WhipDetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("WhipDetectSphere"));
	WhipDetectSphere->SetupAttachment(RootComponent);
	WhipDetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}


void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	if (WhipDetectSphere)
	{
		WhipDetectSphere->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnSpherePassCharacter);	
	}
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()); OwnerCharacter)
	{
		if (ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller); OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUSeServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitCom, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if (bUSeServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && OwnerCharacter->GetEquippedWeapon() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileScoreRequest(HitCharacter, TraceStart, InitialVelocity, OwnerController->GetServerTime() - OwnerController->SingleTripTime, OwnerCharacter->GetEquippedWeapon());
			}
		}
	}
	
	if (WhipDetectSphere)
	{
		WhipDetectSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	}
	
	Super::OnHit(HitCom, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::OnSpherePassCharacter(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	ACharacter* PassingCharacter = Cast<ACharacter>(OtherActor);
	if (OwnerCharacter == PassingCharacter) return;
	if (PassingCharacter)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WhipSoundCue, GetActorLocation());
	}
}
