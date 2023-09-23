// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumOfPlayers == PlayersToWait)
	{
		
		if (UWorld* World = GetWorld(); World)
		{
			bUseSeamlessTravel = false;
			World->ServerTravel(TravelMapPath);
		}
	}
}
