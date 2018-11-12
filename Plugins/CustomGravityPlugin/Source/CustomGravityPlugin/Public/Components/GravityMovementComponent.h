
#pragma once
#include "Kismet/KismetSystemLibrary.h"
#include "CustomGravityManager.h"
#include "GravityMovementComponent.generated.h"


UCLASS()
class CUSTOMGRAVITYPLUGIN_API UGravityMovementComponent : public UFloatingPawnMovement
{
	GENERATED_BODY()

public:
	/**
	* Default UObject constructor.
	*/
	UGravityMovementComponent();

	//Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//End UActorComponent Interface


	//BEGIN UMovementComponent Interface
	virtual bool IsMovingOnGround() const override;
	virtual bool IsFalling() const override;
	virtual void StopMovementImmediately() override;
	//END UMovementComponent Interface

	/**
	* Called when the collision capsule touches another primitive component
	* Handles physics interaction logic	*/
	UFUNCTION()
		virtual void CapsuleHited(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);

	virtual void UpdateCapsuleRotation(float DeltaTime, const FVector& TargetUpVector, float RotationSpeed);
	virtual void ApplyGravity(const FVector& Force, bool bAllowSubstepping, bool bAccelChange);
	virtual void DoJump(FVector ForwardsDir);
	virtual void DoSprint();
	virtual void DoStopSprint();
	virtual void EnableDebuging();
	virtual void DisableDebuging();


	UCharacterMovementComponent*  a;

	/**
	*Custom Gravity Scale.
	*Gravity is multiplied by this amount for the Component Owner (Pawn).
	*In DefaultGravity case : 0 = No Gravity , Other value than 0 = Default Gravity is enabled
	*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		float GravityScale;

	/** If true, Pawn can jump. */
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite, DisplayName = "Can Jump")
		bool bCanJump;

	/** Desired jump height */
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		float JumpHeight = 300.f;

	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		float JumpDistance = 300.f;

	/** Maximum acceptable distance for Gravity pawn capsule/sphere to walk above a surface. */
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float GroundHitToleranceDistance;

	/** When sprinting, multiplier applied to Max Walk Speed */
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float SpeedBoostMultiplier;

	/**
	* When falling, amount of lateral movement control available to the character.
	* 0 = no control, 1 = full control at max speed of MaxWalkSpeed.
	*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float AirControlRatio;

	/** When falling, amount of  time before switch from StandingOrientation to FallingOrientation.*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float GravitySwitchDelay;

	/** If true, set the pawn's velocity to Zero before changing the gravity direction */
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		bool bResetVelocityOnGravitySwitch;

	/**Determine pawn's vertical orientation when is moving on ground*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EVerticalOrientation::Type> StandingVerticalOrientation;

	/**Determine pawn's vertical orientation when is falling*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EVerticalOrientation::Type> FallingVerticalOrientation;

	/**
	*Orientation Settings for each gravity mode:
	*-IsInstant : If true , the orientation is instant;
	*-RotationInterpSpeed : Orientation speed , if IsInstant is false;
	*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		FOrientationSettings OrientationSettings;

	/**Traces Debug Draw Type*/
	UPROPERTY(Category = "Gravity Movement Component : General Settings", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EDrawDebugTrace::Type> DebugDrawType;

	/** Gravity Type
	* Used if Vertical Orientation is set to "Gravity Direction"
	*/
	UPROPERTY(Category = "Gravity Movement Component : Custom Gravity", EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EGravityType::Type> CustomGravityType;

	/** Custom Gravity Information , if "Custom Gravity Type" is set to "Custom Gravity".*/
	UPROPERTY(Category = "Gravity Movement Component : Custom Gravity", EditAnywhere, BlueprintReadWrite)
		FGravityInfo CustomGravityInfo;

	/**Planet Actor Reference .
	* Needed when "Custom Gravity Type" is set to "Point Gravity"
	* If "Point Gravity" is selected and "Planet Actor" is null , No gravity will be applied.
	*/
	UPROPERTY(Category = "Gravity Movement Component : Custom Gravity", EditAnywhere, BlueprintReadWrite)
		APlanetActor* PlanetActor;

	/** Surface Based Gravity Information , if Vertical Orientation is set to "Surface Normal".*/
	UPROPERTY(Category = "Gravity Movement Component : Surface Based Gravity", EditAnywhere, BlueprintReadWrite)
		FGravityInfo SurfaceBasedGravityInfo;

	/** If enabled, Gravity pawn's capsule hit results with be used instead of trace hit results. */
	UPROPERTY(Category = "Gravity Movement Component : Surface Based Gravity", EditAnywhere, BlueprintReadWrite)
		bool bUseCapsuleHit;
	/**
	* Editable if UseCapsuleHit is set to true .
	* Trace shape used to test the surface the Gravity pawn is standing on .
	*/
	UPROPERTY(Category = "Gravity Movement Component : Surface Based Gravity", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "!bUseCapsuleHit"))
		TEnumAsByte<ETraceShape::Type> TraceShape;

	/**Trace Collision Channel .
	* Default value "Pawn" same as the capsule collision objectType.
	* Useful to change it from "Pawn" , if we want to not follow some surfaces normal even if the capsule is standing on "
	*/
	UPROPERTY(Category = "Gravity Movement Component : Surface Based Gravity", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "!bUseCapsuleHit"))
		TEnumAsByte<ECollisionChannel> TraceChannel;

	/**
	* Trace Shape Scale (between 0 and 1) .
	* 0.0f means shape radius/extent equal to 0.0f
	* 1.0 means shape radius/extent equal to Gravity pawn's capsule radius.
	*/
	UPROPERTY(Category = "Gravity Movement Component : Surface Based Gravity", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), meta = (editcondition = "!bUseCapsuleHit"))
		float TraceShapeScale;

	/** If enabled, the player will interact with physics objects when walking into them. */
	UPROPERTY(Category = "Gravity Movement Component : Physics Interaction", EditAnywhere, BlueprintReadWrite)
		bool bEnablePhysicsInteraction;

	/** Force to apply to physics objects that are touched by the player. */
	UPROPERTY(Category = "Gravity Movement Component : Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		float HitForceFactor = 0.25f;

	/** If enabled, the TouchForceFactor is applied per kg mass of the affected object. */
	UPROPERTY(Category = "Gravity Movement Component : Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		bool bHitForceScaledToMass = true;

	/** If enabled, the TouchForceFactor is applied per kg mass of the affected object. */
	UPROPERTY(Category = "Gravity Movement Component : Physics Interaction", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "bEnablePhysicsInteraction"))
		bool bAllowDownwardForce = true;

	/** Information about the surface the Gravity pawn is standing on. */
	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		FHitResult CurrentStandingSurface;

	/** Information about the surface the Gravity pawn is standing on (Updated only when is moving on ground and VericalOrientation is based on surface Normal. */
	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		FHitResult CurrentTracedSurface;

	/** Information about the surface the pawn is standing on (Updated only when is moving on ground and VericalOrientation is based on surface Normal. */
	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		FHitResult CapsuleHitResult;

	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		class AActor* StandingOnActor;

	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsStandingOnPlanet;

	UPROPERTY(Category = "Gravity Movement Component", VisibleInstanceOnly, BlueprintReadOnly)
		bool bIsInAir;

	/* Change Immediately Gravity Info */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		void RequestGravityImmediateUpdate();

	/** Update the Gravity pawn that owns this component. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		void SetComponentOwner(class AGravityPawn* Owner);

	/**Update Current Planet Reference*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		void SetCurrentPlanet(APlanetActor* NewPlanetActor);

	/**Set Current Planet Reference to null*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		void ClearPlanet();

	/** Get the current applied gravity power.
	* Equals to Gravity Power multiplied by Gravity Scale.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		float GetGravityPower() const;

	/** Returns current applied gravity direction. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		FVector GetGravityDirection() const;

	/** Returns APlanetActor reference. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		APlanetActor* GetCurrentPlanet() const;

	/** If Gravity Pawn is sprinting or not. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		virtual bool IsSprinting() const;


	/**
	* Get the falling velocity.
	* Capsule component velocity  projected on -CurrentGravityDirection.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		FVector GetFallingVelocity() const;

	/**
	* Get movement velocity.
	* Gravity movement component velocity  projected on horizontal plane (plane normal = Capsule up vector).
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		FVector GetMovementVelocity() const;

	/**
	* Get current falling speed.
	* Equal Falling Velocity's length multiplied by direction (float)
	* if direction > 0 => jumping
	* if direction < 0 => falling
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		float GetFallingSpeed() const;

	/**
	* Get current falling speed.
	* Equal movement velocity's length
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		float GetCurrentWalkSpeed() const;

	/**
	* Get how much time the pawn is falling (in seconds)
	* when moving on ground , it is set to 0;
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|GravityMovementComponent")
		float GetInAirTime() const;

	FGravityInfo CurrentGravityInfo;

protected:

	/**The Updated component*/
	UCapsuleComponent* CapsuleComponent;

	/** Gravity movement component owner */
	class AGravityPawn* PawnOwner;

private:

	FOrientationInfo CurrentOrientationInfo;

	float CurrentPlanetDistance;

	bool bDebugIsEnabled;

	FRotator CurrentCapsuleRotation;

	float TimeInAir;

	bool bCanResetGravity;

	float CurrentTraceShapeScale;

	float LastWalkSpeed;

	bool bIsSprinting;

	bool bIsJumping;

	bool bRequestImmediateUpdate;

};
