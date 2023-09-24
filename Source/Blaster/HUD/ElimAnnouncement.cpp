// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName)
{
	FString KillerAnnouncement = FString::Printf(TEXT("%s"), *AttackerName);
	FString VictimAnnouncement = FString::Printf(TEXT("%s"), *VictimName);
	if (KillerText)
	{
		KillerText->SetText(FText::FromString(KillerAnnouncement));	
		VictimText->SetText(FText::FromString(VictimAnnouncement));	
	}
}
