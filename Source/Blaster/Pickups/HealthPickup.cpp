#include "HealthPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"


AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComp, OtherBodyIndex, FromSweep, SweepResult);
	
	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor); BlasterCharacter)
	{
		if (UBuffComponent* Buff = BlasterCharacter->GetBuffComponent(); Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}
	SpawnDestroyEffectAttached(Cast<ABlasterCharacter>(OtherActor));
	Destroy();
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHealthPickup::Destroyed()
{
	APickup::Destroyed();
	PlayDestroySound();
}

