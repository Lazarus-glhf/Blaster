#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Blaster.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectile::AProjectile()
{	
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
}

#if WITH_EDITOR
void AProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectile, ProjectileSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
			ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;
		}
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectile, ProjectileGravity))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->ProjectileGravityScale = ProjectileGravity / -980.f;
		}
	}
}
#endif

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();
	
	// 慢速 Projectile 子类应当在客户端也注册碰撞事件，并忽略使用者
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	/*
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * ProjectileSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.OverrideGravityZ = ProjectileGravity;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);
	
	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, GetActorLocation(), GetActorRotation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::ApplyExplodeDamage()
{
	const APawn* FiringPawn = GetInstigator();
	
	// Damage only applied on server
	if (FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController(); FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				MinDamage,
				GetActorLocation(),
				InnerRadius,
				OuterRadius,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
			);
		}
	}
}

void AProjectile::SetUpDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
	TrailDestroyTimer,
	this,
	&AProjectile::DestroyTimerFinished,
	DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}
