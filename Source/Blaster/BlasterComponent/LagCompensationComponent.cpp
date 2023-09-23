#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Weapon/Weapon.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// FFramePackage Package;
	// SaveFramePackage(Package);
	// ShowFramePackage(Package, FColor::Red);
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0, 1);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Character = OlderFrame.Character;
	InterpFramePackage.Time = HitTime;
	
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (Package.Character == nullptr) return FServerSideRewindResult();
	ABlasterCharacter* HitCharacter = Package.Character;

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	// Enable collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 确保命中
	if (UWorld* World = GetWorld(); World)
	{
		FHitResult ConfirmHitResult;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if (ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else // didn't hit the head, check the rest of the boxes
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Package : FramePackages)
	{
		if (Package.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Package : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Package.Character;
		CacheBoxPositions(Package.Character, CurrentFrame);
		MoveBoxes(Package.Character, Package);
		EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);

		// Enable collision for the head first
		UBoxComponent* HeadBox = Package.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	UWorld* World = GetWorld();
	// check for head shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 确保命中
		if (World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					++ShotgunResult.HeadShots[BlasterCharacter];
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// enable collision for all boxes, then disable for head box
	for (auto& Package : FramePackages)
	{
		for (auto& HitBoxPair : Package.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Package.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 确保命中
		if (World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					++ShotgunResult.BodyShots[BlasterCharacter];
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);	
	}
	
	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(const ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	const bool bReturn = HitCharacter == nullptr ||
	HitCharacter->GetLagCompensationComponent() == nullptr ||
	HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
	HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();
	
	// Frame package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory; // HitCharacter's frame history
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime) // 客户端延迟过大
	{
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime) // 链表尾部节点的时间正好等于 HitTime
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	// 从链表头部开始向后遍历，直到 OlderTime < HitTime < YoungerTime
	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// 对 Frame History 中 HitTime 两边的节点进行插值
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	
	FrameToCheck.Time = HitTime;
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (HitCharacter && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), ComponentOwnerCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	if (DamageCauser == nullptr) return;
	
	FShotgunServerSideRewindResult ConfirmResult = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (const auto& HitCharacter : HitCharacters)
	{
		const float HeadShotDamage = ConfirmResult.HeadShots[HitCharacter] * DamageCauser->GetDamage();
		const float BodyShotDamage = ConfirmResult.BodyShots[HitCharacter] * DamageCauser->GetDamage();
		const float TotalDamage = HeadShotDamage + BodyShotDamage;

		UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, ComponentOwnerCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ServerSaveFramePackage();
}

void ULagCompensationComponent::ServerSaveFramePackage()
{
	if (ComponentOwnerCharacter == nullptr || !ComponentOwnerCharacter->HasAuthority()) return;
	
	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddHead(ThisFrame);
	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	while (HistoryLength > MaxRecordTime)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}
	// ShowFramePackage(ThisFrame, FColor::Red);
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	ComponentOwnerCharacter = ComponentOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : ComponentOwnerCharacter;
	if (ComponentOwnerCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = ComponentOwnerCharacter;
		for (auto& BoxPair : ComponentOwnerCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(FName(BoxPair.Key), BoxInformation);
		}
	}
}
