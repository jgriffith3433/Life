
#include "CustomGravityPluginPrivatePCH.h"

UGravityMovementComponent::UGravityMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGravityMovementComponent::SetComponentOwner(class AGravityPawn* Owner)
{
	PawnOwner = Owner;
}

void UGravityMovementComponent::CapsuleHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	/*CapsuleHitResult = Hit;

	const float FallingSpeed = FMath::Abs(GetFallingSpeed());

	if (FallingSpeed > 100.0f)
	{
		FVector CurrentVelocity = CapsuleComponent->GetComponentVelocity();
		CurrentVelocity = CapsuleComponent->GetComponentTransform().InverseTransformVector(CurrentVelocity);
		CurrentVelocity.Z = 0.0f;
		CurrentVelocity = CapsuleComponent->GetComponentTransform().TransformVector(CurrentVelocity);
		CapsuleComponent->SetPhysicsLinearVelocity(CurrentVelocity);
	}

	const float OnGroundHitDot = FVector::DotProduct(HitNormal, CapsuleComponent->GetUpVector());

	if (OnGroundHitDot > 0.75f)
	{
		bIsJumping = false;
	}


	if (!bEnablePhysicsInteraction)
	{
		return;
	}

	if (OtherComp != NULL && OtherComp->IsAnySimulatingPhysics())
	{
		const FVector OtherLoc = OtherComp->GetComponentLocation();
		const FVector Loc = CapsuleComponent->GetComponentLocation();
		FVector ImpulseDir = (OtherLoc - Loc).GetSafeNormal();
		ImpulseDir = FVector::VectorPlaneProject(ImpulseDir, -CurrentGravityInfo.GravityDirection);
		ImpulseDir = (ImpulseDir + GetMovementVelocity().GetSafeNormal()) * 0.5f;
		ImpulseDir.Normalize();


		float TouchForceFactorModified = HitForceFactor;

		if (bHitForceScaledToMass)
		{
			FBodyInstance* BI = OtherComp->GetBodyInstance();
			TouchForceFactorModified *= BI ? BI->GetBodyMass() : 1.0f;
		}

		float ImpulseStrength = GetMovementVelocity().Size() * TouchForceFactorModified;

		FVector Impulse = ImpulseDir * ImpulseStrength;
		float dot = FVector::DotProduct(HitNormal, CapsuleComponent->GetUpVector());

		if (dot > 0.99f && !bAllowDownwardForce)
		{
			return;
		}

		OtherComp->AddImpulseAtLocation(Impulse, HitLocation);
	}*/
}

