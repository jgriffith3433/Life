// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifeCharacter.h"
#include "LifePlayerController.h"
#include "LifeGameMode.h"





ALifeGameMode::ALifeGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/BP_LifeCharacter"));
	DefaultPawnClass = PlayerPawnOb.Class;*/
}

class ALifeCharacter* ALifeGameMode::GetNewLifeCharacter(FTransform SpawnTransform, FActorSpawnParameters SpawnInfo)
{
	return GetWorld()->SpawnActor<ALifeCharacter>(DefaultPawnClass, SpawnTransform, SpawnInfo);
}

UClass* ALifeGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return NULL;
}

void ALifeGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ALifeGameMode::BeginPlay()
{
	APlayerStart* FoundPlayerStart = nullptr;

	TArray<APlayerStart*> StartPoints;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		else
		{
			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			StartPoints.Add(PlayerStart);
		}
	}
	if (FoundPlayerStart == nullptr)
	{
		if (StartPoints.Num() > 0)
		{
			FoundPlayerStart = StartPoints[FMath::RandRange(0, StartPoints.Num() - 1)];
		}
	}

	FTransform Transform = FTransform(FRotator(0, FoundPlayerStart->GetActorRotation().Yaw, 0), FoundPlayerStart->GetActorLocation());
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map

	ALifeCharacter* LifeCharacter = GetNewLifeCharacter(Transform, SpawnInfo);
	ALifePlayerController* controller = Cast<ALifePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (controller)
	{
		controller->Possess(LifeCharacter);
		controller->SwitchToCharacterCamera();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No controller found on begin play"));
	}
	Super::BeginPlay();
}