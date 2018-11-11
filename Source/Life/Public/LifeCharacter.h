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
	ALifeCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;
	virtual void AddForwardMovementInput(float ScaleValue = 1.0f, bool bForce = false) override;
	virtual void AddRightMovementInput(float ScaleValue = 1.0f, bool bForce = false) override;
	virtual void Jump() override;


protected:
	FTimerHandle SlowStepHandle;
	FTimerHandle FastStepHandle;
	FTimerHandle JumpReadyHandle;
	FTimerHandle JumpFinishHandle;

	float ForwardLastMovementInputValue;
	float RightLastMovementInputValue;
	bool bCanStep;
	bool bStartingJump;
	float SteppingSpeed;

	FVector CurrentJumpDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool bJumping;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float TimeOnOneFoot;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float SlowSteppingSpeed;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float FastSteppingSpeed;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float TimeBeforeJump;

	UPROPERTY(EditAnywhere, Category = "Animation")
		float JumpAnimationTime;

	UFUNCTION()
		void OnStep();
	UFUNCTION()
		void OnStepFinish();
	UFUNCTION()
		void OnJumpReady();
	UFUNCTION()
		void OnJumpFinish();
};
