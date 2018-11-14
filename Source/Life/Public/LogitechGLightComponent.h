

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LogitechGLightComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIFE_API ULogitechGLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULogitechGLightComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable, Category = LogitechG)
		void SkyLighting(const int redL, const int greenL, const int blueL);
		
	
};
