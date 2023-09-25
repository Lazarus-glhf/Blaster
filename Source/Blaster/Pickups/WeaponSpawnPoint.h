#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

UCLASS()
class BLASTER_API AWeaponSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AWeaponSpawnPoint();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void StartSpawnPickupTimer();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> PickupClasses;

	UPROPERTY()
	AWeapon* SpawnedWeapon;

	void SpawnWeapon();
	void SpawnWeaponTimerFinished();

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Pad;

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* PadEffect;
	
	FTimerHandle SpawnWeaponTimer;

	UPROPERTY(EditAnywhere)
	float SpawnWeaponTimeMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnWeaponTimeMax;

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 40.f;

	float CountTime = 0.f;
};
