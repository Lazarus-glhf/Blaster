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
	AController* InstigaterController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigaterController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (HitCharacter && InstigaterController)
		{
			if (HasAuthority() && !bUseServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					Damage,
					InstigaterController,
					this,
					UDamageType::StaticClass()
				);	
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(InstigaterController) : BlasterOwnerPlayerController;
				if (BlasterOwnerPlayerController && BlasterOwnerCharacter && HitCharacter->GetLagCompensationComponent())
				{
					BlasterOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
						HitCharacter,
						Start,
						HitTarget,
						BlasterOwnerPlayerController->GetServerTime() - BlasterOwnerPlayerController->SingleTripTime,
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
		DrawDebugSphere(GetWorld(), BeamEnd, 10.f, 10, FColor::Red, true);

		// Spawn trail fx
		if (TrailParticles)
		{
			UNiagaraComponent* TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, TrailParticles, TraceStart);
			TrailComponent->SetVariableVec3(FName("ImpactPoint"), BeamEnd);
			TrailComponent->SetVariableVec3(FName("InitialSpeed"), (BeamEnd - TraceStart).GetSafeNormal() * TraceSpeed);
		}
	}
}
