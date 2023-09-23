#include "HitScanWeapon.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
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
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigatorController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		// Apply damage 仅服务器执行
		ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (HitCharacter && InstigatorController)
		{
			const bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);	
			}
			// 若非服务器则请求服务器上的 LagCompensation 执行 ApplyDamage
			if (!HasAuthority() && bUseServerSideRewind)
			{
				OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : OwnerCharacter;
				OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : OwnerController;
				if (OwnerController && OwnerCharacter && HitCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
				{
					OwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
						HitCharacter,
						Start,
						HitTarget,
						OwnerController->GetServerTime() - OwnerController->SingleTripTime,
						this
					);
				}
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
