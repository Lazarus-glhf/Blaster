#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;

	void EquipWeapon(AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	AWeapon* EquippedWeapon;
		
};
