#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"


AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		{
			if (FiringController)
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
	
	Super::OnHit(HitCom, OtherActor, OtherComp, NormalImpulse, Hit);
}
