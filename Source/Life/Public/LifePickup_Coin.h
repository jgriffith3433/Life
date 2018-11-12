// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LifePickup.h"
#include "LifeCharacter.h"
#include "LifePickup_Coin.generated.h"

/**
 * 
 */
UCLASS()
class LIFE_API ALifePickup_Coin : public ALifePickup
{
	GENERATED_UCLASS_BODY()
protected:

	/** give pickup */
	virtual void GivePickupTo(ALifeCharacter* LifeCharacter) override;
};
