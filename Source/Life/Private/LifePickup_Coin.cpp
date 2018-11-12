// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifePickup_Coin.h"
#include "LifePlayerController.h"
#include "LifeGameMode.h"



ALifePickup_Coin::ALifePickup_Coin(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ALifePickup_Coin::GivePickupTo(ALifeCharacter* LifeCharacter)
{
	if (LifeCharacter)
	{
		ALifePlayerController* LifePC = Cast<ALifePlayerController>(LifeCharacter->Controller);
		if (LifePC)
		{
			LifePC->PickedUpCoins.Add(this);
		}
	}
}
