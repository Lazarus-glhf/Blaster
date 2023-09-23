#include "SpeedBuffPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ASpeedBuffPickup::ASpeedBuffPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpeedBuffPickup::Destroyed()
{
	AActor::Destroyed();
	PlayDestroySound();
}

void ASpeedBuffPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpeedBuffPickup::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComp, OtherBodyIndex, FromSweep, SweepResult);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor); BlasterCharacter)
	{
		if (UBuffComponent* Buff = BlasterCharacter->GetBuffComponent(); Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	SpawnDestroyEffectAttached(Cast<ABlasterCharacter>(OtherActor));
	Destroy();
}

void ASpeedBuffPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

