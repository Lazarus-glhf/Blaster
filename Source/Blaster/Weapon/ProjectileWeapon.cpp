// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket =  GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		const FTransform SocketTransform =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		const FRotator TargetRotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority()) // Server
			{
				if (InstigatorPawn->IsLocallyControlled()) // 服务器控制，生成复制的非 SSR Actor
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUSeServerSideRewind = false;
					SpawnedProjectile->SetDamage(Damage);
				}
				else // 非服务器控制的，生成非复制的 SSR Actor
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUSeServerSideRewind = true;
				}
			}
			else // Client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) // 客户端控制，生成非复制的启用 SSR 的 Actor
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUSeServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->ProjectileSpeed;
					SpawnedProjectile->SetDamage(Damage);
				}
				else // 非客户端控制，生成非复制的不启用 SSR 的 Actor
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
					SpawnedProjectile->bUSeServerSideRewind = false;
				}
			}
		}
		else // weapon 不启用 SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
				SpawnedProjectile->bUSeServerSideRewind = false;
				SpawnedProjectile->SetDamage(Damage);
			}
		}
	}
}
