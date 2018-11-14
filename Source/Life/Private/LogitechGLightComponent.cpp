

#include "Life.h"
#include "LogitechGLightComponent.h"
#include "ILogitechG.h"


// Sets default values for this component's properties
ULogitechGLightComponent::ULogitechGLightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	FModuleManager::LoadModuleChecked<IModuleInterface>("LogitechG");
	if (ILogitechG::IsAvailable())
	{
		ILogitechG::Get().LedInit();
	}
}




// Called when the game starts
void ULogitechGLightComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULogitechGLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULogitechGLightComponent::SkyLighting(const int redL, const int greenL, const int blueL)
{
	ILogitechG::Get().LedSetLighting(redL, greenL, blueL);
}

