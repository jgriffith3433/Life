// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifeCharacter.h"
#include "LifePlayerController.h"
#include "LifePlayerCameraManager.h"



ALifePlayerCameraManager::ALifePlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	TeleportingFOV = 60.0f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void ALifePlayerCameraManager::UpdateCamera(float DeltaTime)
{
	ALifePlayerController* LifePlayerController = PCOwner ? Cast<ALifePlayerController>(PCOwner) : NULL;

	if (LifePlayerController)
	{
		if (LifePlayerController->IsTeleporting())
		{
			DefaultFOV = FMath::FInterpTo(DefaultFOV, TeleportingFOV, DeltaTime, 20.0f);
		}
		else
		{
			DefaultFOV = FMath::FInterpTo(DefaultFOV, NormalFOV, DeltaTime, 20.0f);
		}
	}

	Super::UpdateCamera(DeltaTime);
}
