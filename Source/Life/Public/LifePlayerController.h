// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LifePickup_Coin.h"
#include "LifeTeleporter.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "LifePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LIFE_API ALifePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ALifePlayerController(const FObjectInitializer& ObjectInitializer);

	TArray<ALifePickup_Coin*> PickedUpCoins;
	ALifeCharacter* LifeCharacter;

	virtual void SetupInputComponent() override;
	virtual void SetPawn(APawn* InPawn) override;

	void SwitchToExternalCamera(ACameraActor* CameraActor);
	void SwitchToCharacterCamera();
	void TeleportCharacter(ALifeTeleporter* LifeTeleporter);
	bool IsTeleporting();

	void OnInputMoveForward(float AxisValue);
	void OnInputMoveRight(float AxisValue);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bCanMove;

private:
	FTimerHandle StartTeleportHandle;
	FTimerHandle TeleportHandle;
	ALifeTeleporter* TeleporterDestination;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bIsTeleporting;

	UFUNCTION()
		void FinishTeleporting();
	UFUNCTION()
		void StartTeleporting();
};
