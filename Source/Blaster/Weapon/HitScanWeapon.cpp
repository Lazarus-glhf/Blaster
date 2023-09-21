#include "HitScanWeapon.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AHitScanWeapon::AHitScanWeapon()
{
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

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

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
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, FireHit.ImpactPoint);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	if (UWorld* World = GetWorld(); World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25; // 延伸 LineTrace 的距离确保命中
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		// Spawn trail fx
		if (TrailParticles)
		{
			UNiagaraComponent* TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, TrailParticles, TraceStart);
			TrailComponent->SetVariableVec3(FName("ImpactPoint"), BeamEnd);
			TrailComponent->SetVariableVec3(FName("InitialSpeed"), (BeamEnd - TraceStart).GetSafeNormal() * TraceSpeed);
		}
	}
}
