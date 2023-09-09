// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	CasingMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	ShellEjectionImpulse = 10.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	// TODO Random force maybe?
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse * FMath::FRandRange(0.5, 2));
}

void ACasing::OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bHitted)
	{
		bHitted = true;
		if (ShellSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
		}
	}
}

void ACasing::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ACasing::DestroyTimerFinished,
		DestroyDelay
		);
}

void ACasing::DestroyTimerFinished()
{
	Destroy();
}
