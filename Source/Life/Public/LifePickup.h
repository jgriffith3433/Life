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

	/** hide the pickup */
	void HidePickup();

	/** give pickup */
	UFUNCTION(BlueprintCallable)
		virtual void GivePickup();

	/** deactivate the pickup */
	UFUNCTION(BlueprintCallable)
		void DeactivatePickup();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Effects)
		UStaticMeshComponent* StaticMesh;

	/** Static mesh associated with this pickup. */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float TimeBeforeHide;
		
	bool bCanPickup;

private:
	FTimerHandle HidePickupHandle;
};
