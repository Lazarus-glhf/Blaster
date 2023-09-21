#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AHitScanWeapon();
	virtual void Fire(const FVector& HitTarget) override;
	
protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;
	
	UPROPERTY(EditAnywhere)
	float Damage = 15.f;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactParticles;

private:
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailParticles;

	UPROPERTY(EditAnywhere)
	float TraceSpeed = 60000;
};
