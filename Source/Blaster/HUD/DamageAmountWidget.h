// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageAmountWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UDamageAmountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetDamageText(float Damage);
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DamageText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* DamageTextAnim;

	float TotalDamage = 0;
};
