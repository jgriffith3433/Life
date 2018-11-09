// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GravityCharacter.h"
#include "LifeCharacter.generated.h"


/**
 * Gravity character
 */
UCLASS(config = Game, BlueprintType)
class ALifeCharacter : public AGravityCharacter
{
	GENERATED_BODY()
public:
	/** Default UObject constructor. */
	ALifeCharacter(const FObjectInitializer& ObjectInitializer);
	
};
