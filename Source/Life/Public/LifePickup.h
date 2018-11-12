#pragma once

#include "Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "LifePickup.generated.h"

// Base class for pickup objects that can be placed in the world
UCLASS(abstract)
class ALifePickup : public AActor
{
	GENERATED_UCLASS_BODY()

	/** pickup on touch */
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;
	virtual void BeginPlay() override;

protected:

	/** FX component */
	UPROPERTY(VisibleDefaultsOnly, Category=Effects)
	UParticleSystemComponent* PickupPSC;

	/** FX of active pickup */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystemComponent* ActivePSC;

	/** sound played when player picks it up */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	USoundCue* PickupSound;

	/** Static mesh associated with this pickup. */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UStaticMeshComponent* StaticMesh;

	/** Static mesh associated with this pickup. */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float TimeBeforeDestroy;
		
	bool bCanPickup;

	/** give pickup */
	virtual void GivePickupTo(class ALifeCharacter* LifeCharacter);

private:
	FTimerHandle DestroyPickupHandle;

	UFUNCTION()
		void DestroyPickup();
};
