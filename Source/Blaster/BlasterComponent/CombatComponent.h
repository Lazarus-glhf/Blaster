#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

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

	UFUNCTION(Server, Reliable)
	void SwapWeapons();
	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishAttachWeapons();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UFUNCTION()
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	bool bLocallyReloading = false;
	
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	// Called by Server Fires
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	UFUNCTION()
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshairs(float DeltaTime);
	
	void DropEquippedWeapon();
	void AttachActorToCharacterSocket(AActor* ActorToAttach, FName SocketName);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* Weapon);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming;
	
	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

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

	UPROPERTY(EditAnywhere)
	float CrosshairShootingFactorIncreaseFactor = 20.0f;
	UPROPERTY(EditAnywhere)
	float CrosshairShootingFactorDecreaseFactor = 10.0f;
	UPROPERTY(EditAnywhere)
	float CrosshairShootingFactorMax = 3.0f;
	
	float FireCounter = 0;

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

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingARAmmo = 60;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingPistolAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingShotgunAmmo = 16;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingSniperAmmo = 16;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int StartingGrenadeLauncherAmmo = 16;
	
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxGrenades = 4;

	UFUNCTION()
	void OnRep_Grenades();
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	void UpdateHUDGrenades();

public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
};
