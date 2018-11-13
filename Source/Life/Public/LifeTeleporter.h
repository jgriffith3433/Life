// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "LifeTeleporter.generated.h"

UCLASS()
class LIFE_API ALifeTeleporter : public AActor
{
	GENERATED_BODY()
	
public:	
	ALifeTeleporter(const FObjectInitializer& ObjectInitializer);

	/** teleporter on touch */
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;
	virtual void BeginPlay() override;

	/** Teleport destination */
	UPROPERTY(EditDefaultsOnly, Category = Teleporter)
		USceneComponent* TeleportDestinationComponent;

	UPROPERTY(EditDefaultsOnly, Category = Teleporter)
		float TeleportTime;

	UPROPERTY(EditDefaultsOnly, Category = Teleporter)
		float CameraWaitTime;

	UPROPERTY(EditDefaultsOnly, Category = Teleporter)
		float TeleportInactiveTime;

	UPROPERTY(EditAnywhere, Category = Teleporter)
		ACameraActor* TeleporterCamera;

	/** FX component */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystemComponent* TeleportPSC;

protected:


	/** FX of active pickup */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystemComponent* ActivePSC;

	/** sound played when player teleports */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue* TeleportSound;

	/** sound played when player teleports */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue* TeleportReceiveSound;

	/** sound played on begin play */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		USoundCue* TeleporterActiveSound;

	/** Static mesh associated with this teleporter. */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Category = Teleporter)
		ALifeTeleporter* OtherTeleporter;


	/** Play teleport fx */
	virtual void DoTeleport(class ALifeCharacter* LifeCharacter);

	/** Play teleport fx */
	virtual void ReceiveTeleport(class ALifeCharacter* LifeCharacter);

	UFUNCTION()
		void SetCanTeleport();

	UFUNCTION()
		void PlayTeleportReceive();

private:
	bool bCanTeleport;
	FTimerHandle CanTeleportHandle;
	FTimerHandle TeleportReceiveHandle;

};
