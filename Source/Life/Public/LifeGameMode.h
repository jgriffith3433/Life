// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LifeGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LIFE_API ALifeGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
public:
	class ALifeCharacter* GetNewLifeCharacter(FTransform SpawnTransform, FActorSpawnParameters SpawnInfo);

protected:
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;

};
