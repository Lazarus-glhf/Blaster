#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedBuffPickup.generated.h"

UCLASS()
class BLASTER_API ASpeedBuffPickup : public APickup
{
	GENERATED_BODY()

public:
	ASpeedBuffPickup();
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 5.f;

};
