#include "Shotgun.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigatorController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector TraceStart = SocketTransform.GetLocation();

		// Maps hit character to number of times hit
		TMap<ABlasterCharacter*, uint32> HitMap;

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHitResult;
			WeaponTraceHit(TraceStart, HitTarget, FireHitResult);
			
			if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHitResult.GetActor()); BlasterCharacter)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}

			if (ImpactParticles)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, FireHitResult.ImpactPoint, FireHitResult.ImpactNormal.Rotation());
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, FireHitResult.ImpactPoint, .5f, FMath::FRandRange(-.5f, .5f));
			}
		}

		// Apply damage 仅服务器执行
		TArray<ABlasterCharacter*> HitCharacters;
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
				// 若为服务器且未启用 SSR 则直接生成伤害，否则跳过
				if (HasAuthority() && !bUseServerSideRewind)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());	
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
		// 请求服务器 LagCompensation
		if (!HasAuthority() && bUseServerSideRewind)
		{
			OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : OwnerCharacter;
			OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerPawn->GetController()) : OwnerController;
			if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
			{
				OwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(HitCharacters, TraceStart, HitTargets, OwnerController->GetServerTime() - OwnerController->SingleTripTime, this);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumOfBulletBalls; ++i)
	{
		const FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandomVector;
		FVector ToEndLocation = EndLocation - Start;
		ToEndLocation = Start + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size();
		
		HitTargets.Add(ToEndLocation);
	}
}
