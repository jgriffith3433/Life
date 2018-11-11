#pragma once
#include "GravityMovementComponent.h"
#include "GravityPawn.generated.h"


UCLASS()
class CUSTOMGRAVITYPLUGIN_API AGravityPawn : public APawn
{
	GENERATED_BODY()

public:

	/**
	* Default UObject constructor.
	*/
	AGravityPawn(const FObjectInitializer& ObjectInitializer);

	// APawn interface
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End of AActor interface


	virtual void UpdateMeshRotation(float DeltaTime);


	/** Minimum view Pitch, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Pawn : Camera Settings")
		float CameraPitchMin;

	/** Maximum view Pitch, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Pawn : Camera Settings")
		float CameraPitchMax;

	/** Handle jump action. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		virtual void Jump();

	/** Handle sprint action. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		void Sprint();

	/** Handle stopping sprint action. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		void StopSprint();

	/** Called to move Gravity pawn Forward and Backward */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		virtual void AddForwardMovementInput(float ScaleValue = 1.0f, bool bForce = false);

	/** Called to move Gravity pawn Left and Right */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		virtual void AddRightMovementInput(float ScaleValue = 1.0f, bool bForce = false);

	/**
	* Add input (affecting Pitch) to the SpringArm relative rotation.
	* The added value is the product of UpdateRate & ScaleValue.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		void AddCameraPitchInput(float UpdateRate = 1.0f, float ScaleValue = 0.0f);

	/**
	* Add input (affecting Yaw) to the SpringArm relative rotation.
	* The added value is the product of UpdateRate & ScaleValue.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Input", meta = (Keywords = "AddInput"))
		void AddCameraYawInput(float UpdateRate = 1.0f, float ScaleValue = 0.0f);

	/** Called to enable GravityPawn & MovementComponent debugging. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Debuging")
		void EnableDebugging();

	/** Called to disable GravityPawn & MovementComponent debugging. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn|Debuging")
		void DisableDebugging();

	/**Returns Current Forward Movement Direction. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn")
		FVector GetCurrentForwardDirection() const;

	/**Returns Current Right Movement Direction. */
	UFUNCTION(BlueprintCallable, Category = "Pawn|GravityPawn")
		FVector GetCurrentRightDirection() const;

	/** Movement component used for movement. */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UGravityMovementComponent* MovementComponent;

protected:

	/** The CapsuleComponent being used for movement collision (by CharacterMovement).*/
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	/** The camera boom. */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	/** the main camera associated with this Pawn . */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* Camera;

	/** Skeletal mesh associated with this Pawn. */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* PawnMesh;

	/** Gizmo used as debug arrows root component. */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* GizmoRootComponent;

	/**Forward Arrow Component*/
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* ForwardArrowComponent;

	/**Right Arrow Component*/
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* RightArrowComponent;

	/**Up Arrow Component. */
	UPROPERTY(Category = "Gravity Pawn", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* UpArrowComponent;

public:

	/** Returns CapsuleComponent subobject **/
	FORCEINLINE class UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	/** Returns PawnMesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return PawnMesh; }

	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }

	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	/** Returns GravityMovement Component subobject **/
	FORCEINLINE class UGravityMovementComponent* GetMovementComponent() const { return  MovementComponent; }

	/** Returns Gizmo SceneComponent subobject **/
	FORCEINLINE class USceneComponent* GetGizmoRootComponent() const { return  GizmoRootComponent; }

	/** Returns Forward ArrowComponent subobject **/
	FORCEINLINE class UArrowComponent* GetForwardArrowComponent() const { return ForwardArrowComponent; }

	/** Returns Right ArrowComponent subobject **/
	FORCEINLINE class UArrowComponent* GetRightArrowComponent() const { return RightArrowComponent; }

	/** Returns Up ArrowComponent subobject **/
	FORCEINLINE class UArrowComponent* GetUpArrowComponent() const { return  UpArrowComponent; }

protected:

	/**Runtime updated values. */

		/** Current Forward Movement Direction*/
	FVector CurrentForwardDirection;

	/** Current Right Movement Direction*/
	FVector CurrentRightDirection;



};