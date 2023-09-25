// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "CharacterOverlay.h"
#include "GameFramework/PlayerController.h"
#include "Announcement.h"
#include "DamageAmountWidget.h"
#include "ElimAnnouncement.h"
#include "HitCrosshair.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		if (CharacterOverlay)
		{
			CharacterOverlay->AddToViewport();	
		}
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		if (Announcement)
		{
			Announcement->AddToViewport();	
		}
	}
}

void ABlasterHUD::AddElimAnnouncement(const FString& Attacker, const FString& Victim)
{
	OwningController = OwningController == nullptr ? Cast<ABlasterPlayerController>(GetOwningPlayerController()) : OwningController;
	if (OwningController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningController, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Posotion = CanvasSlot->GetPosition();
						FVector2D NewPosition(Posotion.X, Posotion.Y + CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementLifeTime, false);
		}
	}
}

void ABlasterHUD::AddDamageWidget(float Damage)
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && DamageWidgetClass)
	{
		if (DamageWidget)
		{
			DamageWidget->SetDamageText(Damage);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Trying respawn DamageWidget"));
			DamageWidget = CreateWidget<UDamageAmountWidget>(PlayerController, DamageWidgetClass);
			if (DamageWidget)
			{
				DamageWidget->SetDamageText(Damage);
				DamageWidget->AddToViewport();

				FTimerHandle DamageAmountTimer;
				FTimerDelegate DamageAmountDelegate;
				DamageAmountDelegate.BindUFunction(this, FName("DamageAmountTimerFinished"), DamageWidget);
				GetWorldTimerManager().SetTimer(DamageAmountTimer, DamageAmountDelegate, 1.f, false);
			}
		}
	}
}

void ABlasterHUD::ShowHitCrosshair()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (HitCrosshairWidgetClass)
	{
		UHitCrosshair* HitCrosshair = CreateWidget<UHitCrosshair>(PlayerController, HitCrosshairWidgetClass);
		if (HitCrosshair)
		{
			HitCrosshair->AddToViewport();

			FTimerHandle HitCrosshairTimer;
			FTimerDelegate HitCrosshairDelegate;
			HitCrosshairDelegate.BindUFunction(this, FName("HitCrosshairTimerFinished"), HitCrosshair);
			GetWorldTimerManager().SetTimer(HitCrosshairTimer, HitCrosshairDelegate, 0.1f, false);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::DamageAmountTimerFinished(UDamageAmountWidget* WidgetToRemove)
{
	if (WidgetToRemove)
	{
		WidgetToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::HitCrosshairTimerFinished(UHitCrosshair* CrosshairToRemove)
{
	if (CrosshairToRemove)
	{
		CrosshairToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D::Zero(), HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			const FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			const FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			const FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			const FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}


