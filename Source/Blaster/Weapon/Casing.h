// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

	void StartDestroyTimer();
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* CasingMesh;

	/**
	 * @brief 随机旋转 pitch 和 yaw 范围为 -30 ~ 30 角度，随机系数为 0.75 ~ 1.25
	 */
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

	UPROPERTY(EditAnywhere)
	float DestroyDelay = 5.f;

	bool bHitted = false;
	
	FTimerHandle DestroyTimer;
	
	void DestroyTimerFinished();
};
