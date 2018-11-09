// Copyright 2015 Elhoussine Mehnik (Mhousse1247). All Rights Reserved.
//******************* http://ue4resources.com/ *********************//

#pragma once
#include "Kismet/KismetSystemLibrary.h"
#include "GravityMovementComponent.generated.h"


UENUM(BlueprintType)
enum EOrientationInterpolationMode
{
	OIM_RInterpTo 	UMETA(DisplayName = "Rotator Interpolation"),
	OIM_Slerp	UMETA(DisplayName = "Quaternion Slerp")
};

UENUM(BlueprintType)
enum EVerticalOrientation
{
	EVO_GravityDirection 	UMETA(DisplayName = "Gravity Direction"),
	EVO_SurfaceNormal	UMETA(DisplayName = "Surface Normal")
};

UENUM(BlueprintType)
enum ETraceShape
{
	ETS_Sphere	UMETA(DisplayName = "Sphere"),
	ETS_Box	UMETA(DisplayName = "Box"),
	ETS_Line 	UMETA(DisplayName = "Line")
};

/** Enumerates available custom gravity types. */
UENUM(BlueprintType)
enum EGravityType
{
	EGT_Default 	UMETA(DisplayName = "Default Gravity"),
	EGT_Point 	UMETA(DisplayName = "Point Gravity"),
	EGT_Custom 	UMETA(DisplayName = "Custom Gravity"),
	EGT_GlobalCustom 	UMETA(DisplayName = "Global Custom Gravity")
};

/** Type of force applied to a body using Custom Gravity. */
UENUM(BlueprintType)
enum EForceMode
{
	EFM_Acceleration 	UMETA(DisplayName = "Acceleration"),
	EFM_Force 	UMETA(DisplayName = "Force")
};

/** Struct to hold information about the "Gravity Type" . */
USTRUCT(BlueprintType)
struct CUSTOMGRAVITYPLUGIN_API FGravityInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float GravityPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector GravityDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EForceMode> ForceMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bForceSubStepping;

	FGravityInfo()
	{
		GravityPower = 980.0f;
		GravityDirection = FVector(0.0f, 0.0f, -1.0f);
		ForceMode = EForceMode::EFM_Acceleration;
		bForceSubStepping = true;
	}

	FGravityInfo(float NewGravityPower, FVector NewGravityDirection, EForceMode NewForceMode, bool bShouldUseStepping)
	{
		GravityPower = NewGravityPower;
		GravityDirection = NewGravityDirection;
		ForceMode = NewForceMode;
		bForceSubStepping = bShouldUseStepping;
	}

};

USTRUCT(BlueprintType)
struct FOrientationInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		bool bIsInstant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		float RotationInterpSpeed;

	FOrientationInfo()
	{
		bIsInstant = false;
		RotationInterpSpeed = 5.0f;
	}
	FOrientationInfo(bool _isInstant, float _interpSpeed)
	{
		bIsInstant = _isInstant;
		RotationInterpSpeed = _interpSpeed;
	}
};


USTRUCT(BlueprintType)
struct FOrientationSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		TEnumAsByte<EOrientationInterpolationMode> InterpolationMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		FOrientationInfo DefaultGravity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		FOrientationInfo PointGravity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		FOrientationInfo CustomGravity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		FOrientationInfo GlobalCustomGravity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orientation Settings")
		FOrientationInfo SurfaceBasedGravity;

	FOrientationSettings()
		: InterpolationMode(EOrientationInterpolationMode::OIM_Slerp)
	{

	}
};

class APlanetActor;

UCLASS()
class CUSTOMGRAVITYPLUGIN_API UGravityMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UGravityMovementComponent(const FObjectInitializer& ObjectInitializer);

	/**Planet Actor Reference .
	* Needed when "Custom Gravity Type" is set to "Point Gravity"
	* If "Point Gravity" is selected and "Planet Actor" is null , No gravity will be applied.
	*/
	UPROPERTY(Category = "Gravity Movement Component : Custom Gravity", EditAnywhere, BlueprintReadWrite)
		APlanetActor* PlanetActor;

	/**Determine pawn's vertical orientation when is moving on ground*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EVerticalOrientation> StandingVerticalOrientation;

	/**Determine pawn's vertical orientation when is falling*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EVerticalOrientation> FallingVerticalOrientation;

	/**
	*Orientation Settings for each gravity mode:
	*-IsInstant : If true , the orientation is instant;
	*-RotationInterpSpeed : Orientation speed , if IsInstant is false;
	*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		FOrientationSettings OrientationSettings;

	/** Gravity Type
	* Used if Vertical Orientation is set to "Gravity Direction"
	*/
	UPROPERTY(Category = "Gravity Movement Component : Custom Gravity", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EGravityType> CustomGravityType;

	void SetComponentOwner(class AGravityPawn* Owner);

	/**
	* Called when the collision capsule touches another primitive component
	* Handles physics interaction logic	*/
	UFUNCTION()
		virtual void CapsuleHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);
	


protected:
	/** Custom movement component owner */
	class AGravityPawn* PawnOwner;
};
