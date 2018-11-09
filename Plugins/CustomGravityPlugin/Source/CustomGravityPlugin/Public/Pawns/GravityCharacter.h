#pragma once
#include "GravityPawn.h"
#include "GravityCharacter.generated.h"


UCLASS()
class  CUSTOMGRAVITYPLUGIN_API AGravityCharacter : public AGravityPawn
{
	GENERATED_BODY()

public:
	AGravityCharacter(const FObjectInitializer& ObjectInitializer);

};