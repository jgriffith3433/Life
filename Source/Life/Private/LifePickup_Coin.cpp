// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifePickup_Coin.h"
#include "LifePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "LifeGameMode.h"



ALifePickup_Coin::ALifePickup_Coin(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ALifePickup_Coin::BeginPlay()
{
	Super::BeginPlay();
	ALifePlayerController* LifePlayerController = Cast<ALifePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (LifePlayerController)
	{
		LifePlayerController->AddLevelCoin(this);
	}
}

void ALifePickup_Coin::GivePickup()
{
	ALifePlayerController* LifePlayerController = Cast<ALifePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (LifePlayerController)
	{
		LifePlayerController->PickupCoin(this);
	}
}
