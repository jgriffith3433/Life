// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifeCharacter.h"
#include "LifePlayerCameraManager.h"
#include "LifeGameMode.h"
#include "LifePlayerController.h"



ALifePlayerController::ALifePlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ALifePlayerCameraManager::StaticClass();
	bCanMove = true;
	bAutoManageActiveCameraTarget = false;
}


void ALifePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	InputComponent->BindAxis("MoveForward", this, &ALifePlayerController::OnInputMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ALifePlayerController::OnInputMoveRight);

	//InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AShooterPlayerController::OnToggleInGameMenu);
	//InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AShooterPlayerController::OnShowScoreboard);
	//InputComponent->BindAction("Scoreboard", IE_Released, this, &AShooterPlayerController::OnHideScoreboard);
	//InputComponent->BindAction("ConditionalCloseScoreboard", IE_Pressed, this, &AShooterPlayerController::OnConditionalCloseScoreboard);
	//InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &AShooterPlayerController::OnToggleScoreboard);
	
}

void ALifePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (InPawn == NULL)
	{
		bCanMove = false;
		return;
	}
	else
	{
		bCanMove = true;
	}
	if (InPawn->IsA(ALifeCharacter::StaticClass()))
	{
		LifeCharacter = Cast<ALifeCharacter>(InPawn);
	}
}


void ALifePlayerController::OnInputMoveForward(float AxisValue)
{
	if (LifeCharacter && bCanMove)
	{
		LifeCharacter->AddForwardMovement(AxisValue);
	}
}

void ALifePlayerController::OnInputMoveRight(float AxisValue)
{
	if (LifeCharacter && bCanMove)
	{
		LifeCharacter->AddRightMovement(AxisValue);
	}
}

void ALifePlayerController::SwitchToExternalCamera(ACameraActor* CameraActor)
{
	SetViewTarget(CameraActor);
}

void ALifePlayerController::SwitchToCharacterCamera()
{
	if (LifeCharacter)
	{
		SetViewTargetWithBlend(LifeCharacter);
	}
}

bool ALifePlayerController::IsTeleporting()
{
	return bIsTeleporting;
}

void ALifePlayerController::TeleportCharacter(ALifeTeleporter* _TeleporterDestination)
{
	TeleporterDestination = _TeleporterDestination;
	bIsTeleporting = true;
	bCanMove = false;
	if (LifeCharacter)
	{
		LifeCharacter->StopAllAnimMontages();
		LifeCharacter->StopMovement();
		GetWorld()->GetTimerManager().SetTimer(StartTeleportHandle, this, &ALifePlayerController::StartTeleporting, TeleporterDestination->CameraWaitTime, false);
		if (TeleporterDestination->TeleporterCamera)
		{
			SwitchToExternalCamera(TeleporterDestination->TeleporterCamera);
		}
	}
}

void ALifePlayerController::StartTeleporting()
{
	GetWorld()->GetTimerManager().ClearTimer(StartTeleportHandle);
	GetWorld()->GetTimerManager().SetTimer(TeleportHandle, this, &ALifePlayerController::FinishTeleporting, TeleporterDestination->TeleportTime, false);
	UnPossess();
	LifeCharacter->Destroy();
	LifeCharacter = NULL;
}

void ALifePlayerController::FinishTeleporting()
{
	GetWorld()->GetTimerManager().ClearTimer(TeleportHandle);
	bCanMove = true;
	bIsTeleporting = false;
	if (TeleporterDestination)
	{
		UWorld* world = GetWorld();
		if (world)
		{
			AGameModeBase* GameMode = world->GetAuthGameMode();
			if (GameMode)
			{
				ALifeGameMode* LifeGameMode = Cast<ALifeGameMode>(GameMode);
				if (LifeGameMode)
				{
					FTransform TeleporterDestinationTransform = TeleporterDestination->TeleportDestinationComponent->GetComponentTransform();
					
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.Instigator = Instigator;
					SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
					Possess(LifeGameMode->GetNewLifeCharacter(TeleporterDestinationTransform, SpawnInfo));
					GetWorld()->GetTimerManager().SetTimer(TeleportHandle, this, &ALifePlayerController::SwitchToCharacterCamera, TeleporterDestination->CameraWaitTime, false);
				}
			}
		}
	}
}
