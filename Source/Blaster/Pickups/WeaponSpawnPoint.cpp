// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawnPoint.h"
#include "Blaster/Weapon/Weapon.h"
#include "NiagaraComponent.h"


// Sets default values
AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Pad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pad"));
	SetRootComponent(Pad);
	PadEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PadEffect"));
	PadEffect->SetupAttachment(RootComponent);
}

void AWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnWeapon();	
	}
}

void AWeaponSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CountTime += DeltaTime;
	if (SpawnedWeapon)
	{
		SpawnedWeapon->AddActorLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f) * FMath::Abs(FMath::Sin(CountTime)));
	}
}

void AWeaponSpawnPoint::SpawnWeapon()
{
	if (const int32 NumPickupClasses = PickupClasses.Num(); NumPickupClasses > 0)
	{
		const int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		FVector SpawnLocation = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 100.f);
		SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(PickupClasses[Selection], SpawnLocation, GetActorRotation());

		SpawnedWeapon->InitialSpawnPoint(this);
	}
}

void AWeaponSpawnPoint::StartSpawnPickupTimer()
{
	if (SpawnedWeapon)
	{
		SpawnedWeapon = nullptr;
	}
	const float SpawnTime = FMath::FRandRange(SpawnWeaponTimeMin, SpawnWeaponTimeMax);
	GetWorldTimerManager().SetTimer(SpawnWeaponTimer, this, &AWeaponSpawnPoint::SpawnWeaponTimerFinished, SpawnTime);
}

void AWeaponSpawnPoint::SpawnWeaponTimerFinished()
{
	if (HasAuthority())
	{
		SpawnWeapon();
	}
}

