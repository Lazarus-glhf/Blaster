#include "RocketMovementComponent.h"


URocketMovementComponent::URocketMovementComponent()
{
}

void URocketMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Super::HandleImpact(Hit, TimeSlice, MoveDelta);
	// Rockets should not stop; only explode when their CollisionBox detects a hit
}
