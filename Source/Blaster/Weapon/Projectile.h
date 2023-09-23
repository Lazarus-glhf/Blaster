// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Destroyed() override;

	/**
	 *	Used with server-side rewind
	 */
	bool bUSeServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float ProjectileSpeed = 1500.f;

	UPROPERTY(EditAnywhere)
	float ProjectileGravity = -980.f;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override; 
#endif
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UFUNCTION()
	virtual void SetUpDestroyTimer();
	
	UFUNCTION()
	virtual void DestroyTimerFinished();

	UFUNCTION()
	void SpawnTrailSystem();
	
	UPROPERTY(EditAnywhere)
	bool bUseDestroyTimer = true;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere, Category = "Explode")
	float MinDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Explode")
	float InnerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Explode")
	float OuterRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Explode")
	float DamageFalloff = 1.f;

	void ApplyExplodeDamage();
	
private:
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	/**
	 * @brief Projectile 销毁延时
	 */
	UPROPERTY(EditAnywhere)
	float DestroyTime = 0.5f;
	
	FTimerHandle TrailDestroyTimer;
};
