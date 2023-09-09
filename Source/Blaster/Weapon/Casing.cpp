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

	FRotator RandomRotator = FRotator(FMath::FRandRange(-30., 30.), FMath::FRandRange(-30., 30.), 0);
    CasingMesh->AddImpulse(RandomRotator.RotateVector(GetActorForwardVector()) * ShellEjectionImpulse * FMath::FRandRange(0.75, 1.25));
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
