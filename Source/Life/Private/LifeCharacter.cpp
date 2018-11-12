// Fill out your copyright notice in the Description page of Project Settings.


#include "Life.h"
#include "LifeTeleporter.h"
#include "LifePlayerController.h"
#include "LifeCharacter.h"



ALifeCharacter::ALifeCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TimeOnOneFoot = 0.2f;
	bCanStep = false;
	bStartingJump = false;
	SlowSteppingSpeed = 0.2f;
	FastSteppingSpeed = 1.0f;
	TimeBeforeJump = 0.5f;
	JumpAnimationTime = 1.2f;
}

void ALifeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementComponent == NULL) { return; }

	if (MovementComponent->IsMovingOnGround())
	{
		AddMovementInput(CurrentForwardDirection.GetSafeNormal(), ForwardLastMovementInputValue * SteppingSpeed, false);
		AddMovementInput(CurrentRightDirection.GetSafeNormal(), RightLastMovementInputValue * SteppingSpeed, false);
	}
	else
	{
		AddMovementInput(CurrentForwardDirection.GetSafeNormal(), ForwardLastMovementInputValue * MovementComponent->AirControlRatio, false);
		AddMovementInput(CurrentRightDirection.GetSafeNormal(), RightLastMovementInputValue * MovementComponent->AirControlRatio, false);
	}
}

void ALifeCharacter::AddForwardMovement(float ScaleValue)
{
	if (MovementComponent == NULL || !GetWorld()) { return; }

	if (!GetWorld()) { return; }

	const FVector UpDirection = GetActorUpVector();
	const FVector CameraForward = Camera->GetForwardVector();
	const float Dot = FVector::DotProduct(UpDirection, CameraForward);

	if (FMath::Abs(Dot) < 1 - SMALL_NUMBER)
	{
		CurrentForwardDirection = FVector::VectorPlaneProject(CameraForward, GetActorUpVector());
	}

	ForwardLastMovementInputValue = MovementComponent->IsMovingOnGround() ? ScaleValue : ScaleValue * MovementComponent->AirControlRatio;

	if (ForwardLastMovementInputValue != 0.0f)
	{
		if (bCanStep)
		{
			bCanStep = false;
			GetWorld()->GetTimerManager().SetTimer(SlowStepHandle, this, &ALifeCharacter::OnStep, TimeOnOneFoot, false);
		}
	}
	else
	{
		bCanStep = true;
		SteppingSpeed = SlowSteppingSpeed;
		GetWorld()->GetTimerManager().ClearTimer(SlowStepHandle);
		GetWorld()->GetTimerManager().ClearTimer(FastStepHandle);
	}
}

void ALifeCharacter::AddRightMovement(float ScaleValue)
{
	if (MovementComponent == NULL || !GetWorld()) { return; }

	const FVector UpDirection = GetActorUpVector();
	const FVector CameraRight = Camera->GetRightVector();
	const float Dot = FVector::DotProduct(UpDirection, CameraRight);

	if (FMath::Abs(Dot) < 1 - SMALL_NUMBER)
	{
		CurrentRightDirection = FVector::VectorPlaneProject(CameraRight, UpDirection);
	}

	RightLastMovementInputValue = MovementComponent->IsMovingOnGround() ? ScaleValue : ScaleValue * MovementComponent->AirControlRatio;

	if (ForwardLastMovementInputValue != 0.0f)
	{
		if (bCanStep)
		{
			bCanStep = false;
			GetWorld()->GetTimerManager().SetTimer(SlowStepHandle, this, &ALifeCharacter::OnStep, TimeOnOneFoot, false);
		}
	}
	else
	{
		bCanStep = true;
		SteppingSpeed = SlowSteppingSpeed;
		GetWorld()->GetTimerManager().ClearTimer(SlowStepHandle);
		GetWorld()->GetTimerManager().ClearTimer(FastStepHandle);
	}
}

void ALifeCharacter::StopMovement()
{
	ForwardLastMovementInputValue = 0.0f;
	RightLastMovementInputValue = 0.0f;
}

void ALifeCharacter::OnStep()
{
	SteppingSpeed = FastSteppingSpeed;
	GetWorld()->GetTimerManager().ClearTimer(SlowStepHandle);
	GetWorld()->GetTimerManager().SetTimer(FastStepHandle, this, &ALifeCharacter::OnStepFinish, TimeOnOneFoot, false);
}

void ALifeCharacter::OnStepFinish()
{
	bCanStep = true;
	SteppingSpeed = SlowSteppingSpeed;
	GetWorld()->GetTimerManager().ClearTimer(FastStepHandle);
}

void ALifeCharacter::Jump()
{
	if (!bStartingJump)
	{
		bStartingJump = true;
		bJumping = true;
		CurrentJumpDirection = CurrentForwardDirection.GetSafeNormal();
		GetWorld()->GetTimerManager().SetTimer(JumpReadyHandle, this, &ALifeCharacter::OnJumpReady, TimeBeforeJump, false);
	}
}

void ALifeCharacter::OnJumpReady()
{
	bStartingJump = false;

	GetWorld()->GetTimerManager().ClearTimer(JumpReadyHandle);
	if (MovementComponent != NULL)
	{
		GetWorld()->GetTimerManager().SetTimer(JumpFinishHandle, this, &ALifeCharacter::OnJumpFinish, JumpAnimationTime, false);
		MovementComponent->DoJump(CurrentJumpDirection);
	}
}

void ALifeCharacter::OnJumpFinish()
{
	bJumping = false;
	GetWorld()->GetTimerManager().ClearTimer(JumpFinishHandle);
}

void ALifeCharacter::TeleportCharacter(ALifeTeleporter* LifeTeleporter)
{
	ALifePlayerController* LifePlayerController = Cast<ALifePlayerController>(Controller);
	if (LifePlayerController)
	{
		LifePlayerController->TeleportCharacter(LifeTeleporter);
	}
}

void ALifeCharacter::StopAllAnimMontages()
{
	if (PawnMesh && PawnMesh->AnimScriptInstance)
	{
		PawnMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}
