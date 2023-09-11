// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
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

	WhipDetectSphere->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnSpherePassCharacter);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && HasAuthority())
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());	
		}
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
