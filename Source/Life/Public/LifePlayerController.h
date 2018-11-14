// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LifeTeleporter.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "LifePlayerController.generated.h"

class ALifePickup_Coin;

UCLASS()
class LIFE_API ALifePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ALifePlayerController(const FObjectInitializer& ObjectInitializer);

	ALifeCharacter* LifeCharacter;


	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void SetPawn(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
		void SwitchToExternalCamera(ACameraActor* CameraActor);
	UFUNCTION(BlueprintCallable)
		void SwitchToCharacterCamera();

	void TeleportCharacter(ALifeTeleporter* LifeTeleporter);
	bool IsTeleporting();

	void OnInputMoveForward(float AxisValue);
	void OnInputMoveRight(float AxisValue);

	void OnQuitPause();

	void PickupCoin(ALifePickup_Coin* Coin);
	void AddLevelCoin(ALifePickup_Coin* Coin);

	void FinishLevel();

	UFUNCTION(BlueprintImplementableEvent)
		void OnFinishLevel();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bCanMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool bIsPaused;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		int TotalCoinsThisLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		int TotalPickedUpCoinsThisLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TArray<ALifePickup_Coin*> PickedUpCoins;

private:
	FTimerHandle StartTeleportHandle;
	FTimerHandle TeleportHandle;
	ALifeTeleporter* TeleporterDestination;

	TArray<ALifePickup_Coin*> LevelCoins;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bIsTeleporting;

	UFUNCTION()
		void FinishTeleporting();
	UFUNCTION()
		void StartTeleporting();
};
