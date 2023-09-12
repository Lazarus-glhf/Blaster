#include "HitScanWeapon.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"

AHitScanWeapon::AHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigaterController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigaterController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25; // 延伸 LineTrace 的距离确保命中

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			if (FireHit.bBlockingHit)
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				if (BlasterCharacter)
				{
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
							BlasterCharacter,
							Damage,
							InstigaterController,
							this,
							UDamageType::StaticClass()
						);	
					}
				}
				if (ImpactParticles)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						World,
						ImpactParticles,
						HitTarget,
						FireHit.ImpactNormal.Rotation()
					);
				}
			}
		}
		if (TrailParticles)
		{
			UNiagaraComponent* TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				TrailParticles,
				Start
			);
			TrailComponent->SetVariableVec3(FName("ImpactPoint"), HitTarget);
			TrailComponent->SetVariableVec3(FName("InitialSpeed"), (HitTarget - Start).GetSafeNormal() * TraceSpeed);
		}
	}
}

void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHitScanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

