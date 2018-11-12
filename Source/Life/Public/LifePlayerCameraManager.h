// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "LifePlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class LIFE_API ALifePlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()
	
public:
	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float TeleportingFOV;

	/** After updating camera */
	virtual void UpdateCamera(float DeltaTime) override;
	
};
