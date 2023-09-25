// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageAmountWidget.h"

#include "Components/TextBlock.h"

void UDamageAmountWidget::SetDamageText(float Damage)
{
	TotalDamage += Damage;
	if (DamageText)
	{
		FString DamageAmount = FString::Printf(TEXT("%f"), TotalDamage);
		DamageText->SetText(FText::FromString(DamageAmount));
	}
	if (DamageTextAnim)
	{
		PlayAnimation(DamageTextAnim);
	}
}

