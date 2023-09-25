// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	UTexture2D* CrosshairsCenter;
	UPROPERTY()
	UTexture2D* CrosshairsLeft;
	UPROPERTY()
	UTexture2D* CrosshairsRight;
	UPROPERTY()
	UTexture2D* CrosshairsTop;
	UPROPERTY()
	UTexture2D* CrosshairsBottom;

	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> AnnouncementClass;
	
	UPROPERTY()
	class UAnnouncement* Announcement;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> DamageWidgetClass;

	UPROPERTY()
	class UDamageAmountWidget* DamageWidget;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> HitCrosshairWidgetClass;

	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(const FString& Attacker, const FString& Victim);
	void AddDamageWidget(float Damage);
	void ShowHitCrosshair();

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerController* OwningController;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread = FVector2D::ZeroVector, FLinearColor CrosshairColor = FLinearColor::White);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	/**
	 * Elim Announcement
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementLifeTime = 1.5f;
	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);
	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

	UFUNCTION()
	void DamageAmountTimerFinished(UDamageAmountWidget* WidgetToRemove);
	
	UFUNCTION()
	void HitCrosshairTimerFinished(UHitCrosshair* CrosshairToRemove);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
