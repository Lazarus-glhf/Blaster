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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> PickupClasses;

	UPROPERTY()
	AWeapon* SpawnedWeapon;

	void SpawnWeapon();
	void SpawnWeaponTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Pad;
	
	FTimerHandle SpawnWeaponTimer;

	UPROPERTY(EditAnywhere)
	float SpawnWeaponTimeMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnWeaponTimeMax;

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 40.f;

	float CountTime = 0.f;
};
