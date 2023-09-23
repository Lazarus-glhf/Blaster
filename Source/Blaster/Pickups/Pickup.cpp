#include "Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Weapon/WeaponTypes.h"


APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(3.f));
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapTimerFinished, BindOverlapTime);	
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();
	
	PlayDestroySound();
	SpawnDestroyEffect();
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddLocalRotation(FRotator(0.f, PickupTurnRate * DeltaTime, 0.f));
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	
}

void APickup::SpawnDestroyEffect()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());	
	}
}

void APickup::SpawnDestroyEffectAttached(ABlasterCharacter* AttachCharacter)
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			PickupEffect,
			AttachCharacter->GetMesh(),
			FName("pelvis"),
			FVector(),
			AttachCharacter->GetActorUpVector().Rotation() * -1,
			EAttachLocation::Type::KeepRelativeOffset,
			true,
			true,
			ENCPoolMethod::None,
			true
		);	
	}
}

void APickup::PlayDestroySound()
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
}

void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}
