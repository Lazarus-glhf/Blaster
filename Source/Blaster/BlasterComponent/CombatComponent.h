#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000.f

class AWeapon;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Fire();
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION()
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshairs(float DeltaTime);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	 * HUD and crosshairs
	 */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	// TODO 获取自武器；由直接设定为固定值改为渐变
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/**
	 * Aiming and FOV
	 */

	// Field of view not aiming; set to the camera's base FOV in Beginplay
	float DefaultFOV;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	 * Automatic fire
	 */
	FTimerHandle FireTimer;
	
	bool bCanFire = true;
 
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int StartingARAmmo = 30;
	
	void InitializeCarriedAmmo();
};
