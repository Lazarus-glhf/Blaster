// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY(EditAnywhere)
	uint32 PlayersToWait = 2;

	UPROPERTY(EditAnywhere)
	FString TravelMapPath = FString("/Game/Maps/DesertBattleMap?listen");
};
