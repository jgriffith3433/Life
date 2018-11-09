// Fill out your copyright notice in the Description page of Project Settings.
/*=============================================================================
	LifeCharacter.cpp: ALifeCharacter implementation
=============================================================================*/

#include "Life.h"
#include "LifeCharacter.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/DemoNetDriver.h"
#include "Components/CapsuleComponent.h"
#include "GravityMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Animation/AnimInstance.h"


DEFINE_LOG_CATEGORY_STATIC(LogCharacter, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogAvatar, Log, All);

DECLARE_CYCLE_STAT(TEXT("Char OnNetUpdateSimulatedPosition"), STAT_CharacterOnNetUpdateSimulatedPosition, STATGROUP_Character);

FName ALifeCharacter::MeshComponentName(TEXT("CharacterMesh0"));
FName ALifeCharacter::CapsuleComponentName(TEXT("CollisionCylinder"));

ALifeCharacter::ALifeCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		FName ID_Characters;
		FText NAME_Characters;
		FConstructorStatics()
			: ID_Characters(TEXT("Characters"))
			, NAME_Characters(NSLOCTEXT("SpriteCategory", "Characters", "Characters"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Character rotation only changes in Yaw, to prevent the capsule from changing orientation.
	// Ask the Controller for the full rotation if desired (ie for aiming).
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	if (CapsuleComponent)
	{
		CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
		CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		CapsuleComponent->CanCharacterStepUpOn = ECB_No;
		CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
		CapsuleComponent->bCheckAsyncSceneOnMove = false;
		CapsuleComponent->SetCanEverAffectNavigation(false);
		CapsuleComponent->bDynamicObstacle = true;
	}

	bClientCheckEncroachmentOnNetUpdate = true;
	JumpKeyHoldTime = 0.0f;
	JumpMaxHoldTime = 0.0f;
	JumpMaxCount = 1;
	JumpCurrentCount = 0;
	bWasJumping = false;

	AnimRootMotionTranslationScale = 1.0f;

	if (GravityMovementComponent)
	{
		CrouchedEyeHeight = GravityMovementComponent->CrouchedHalfHeight * 0.80f;
	}

	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
	}

	BaseRotationOffset = FQuat::Identity;
}

void ALifeCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (Mesh)
		{
			CacheInitialMeshOffset(Mesh->RelativeLocation, Mesh->RelativeRotation);
		}

		if (GravityMovementComponent && CapsuleComponent)
		{
			GravityMovementComponent->UpdateNavAgent(*CapsuleComponent);
		}

		if (Controller == nullptr && GetNetMode() != NM_Client)
		{
			if (GravityMovementComponent && GravityMovementComponent->bRunPhysicsWithNoController)
			{
				GravityMovementComponent->SetDefaultMovementMode();
			}
		}
	}
}

void ALifeCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void ALifeCharacter::CacheInitialMeshOffset(FVector MeshRelativeLocation, FRotator MeshRelativeRotation)
{
	BaseTranslationOffset = MeshRelativeLocation;
	BaseRotationOffset = MeshRelativeRotation.Quaternion();

#if ENABLE_NAN_DIAGNOSTIC
	if (BaseRotationOffset.ContainsNaN())
	{
		logOrEnsureNanError(TEXT("ALifeCharacter::PostInitializeComponents detected NaN in BaseRotationOffset! (%s)"), *BaseRotationOffset.ToString());
	}
	if (Mesh->RelativeRotation.ContainsNaN())
	{
		logOrEnsureNanError(TEXT("ALifeCharacter::PostInitializeComponents detected NaN in Mesh->RelativeRotation! (%s)"), *Mesh->RelativeRotation.ToString());
	}
#endif
}

void ALifeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
}


void ALifeCharacter::GetSimpleCollisionCylinder(float& CollisionRadius, float& CollisionHalfHeight) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (IsTemplate())
	{
		UE_LOG(LogCharacter, Log, TEXT("WARNING ALifeCharacter::GetSimpleCollisionCylinder : Called on default object '%s'. Will likely return zero size. Consider using GetDefaultHalfHeight() instead."), *this->GetPathName());
	}
#endif

	if (RootComponent == CapsuleComponent && IsRootComponentCollisionRegistered())
	{
		// Note: we purposefully ignore the component transform here aside from scale, always treating it as vertically aligned.
		// This improves performance and is also how we stated the CapsuleComponent would be used.
		CapsuleComponent->GetScaledCapsuleSize(CollisionRadius, CollisionHalfHeight);
	}
	else
	{
		Super::GetSimpleCollisionCylinder(CollisionRadius, CollisionHalfHeight);
	}
}

void ALifeCharacter::UpdateNavigationRelevance()
{
	if (CapsuleComponent)
	{
		CapsuleComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
	}
}

float ALifeCharacter::GetDefaultHalfHeight() const
{
	UCapsuleComponent* DefaultCapsule = GetClass()->GetDefaultObject<ALifeCharacter>()->CapsuleComponent;
	if (DefaultCapsule)
	{
		return DefaultCapsule->GetScaledCapsuleHalfHeight();
	}
	else
	{
		return Super::GetDefaultHalfHeight();
	}
}


UActorComponent* ALifeCharacter::FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const
{
	// If the character has a Mesh, treat it as the first 'hit' when finding components
	if (Mesh && ComponentClass && Mesh->IsA(ComponentClass))
	{
		return Mesh;
	}

	return Super::FindComponentByClass(ComponentClass);
}

void ALifeCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
}

void ALifeCharacter::NotifyJumpApex()
{
	// Call delegate callback
	if (OnReachedJumpApex.IsBound())
	{
		OnReachedJumpApex.Broadcast();
	}
}

void ALifeCharacter::Landed(const FHitResult& Hit)
{
	OnLanded(Hit);

	LandedDelegate.Broadcast(Hit);
}

bool ALifeCharacter::CanJump() const
{
	return CanJumpInternal();
}

bool ALifeCharacter::CanJumpInternal_Implementation() const
{
	// Ensure the character isn't currently crouched.
	bool bCanJump = !bIsCrouched;

	// Ensure that the MovementComponent state is valid
	bCanJump &= GravityMovementComponent &&
		GravityMovementComponent->IsJumpAllowed() &&
		!GravityMovementComponent->bWantsToCrouch &&
		// Can only jump from the ground, or multi-jump if already falling.
		(GravityMovementComponent->IsMovingOnGround() || GravityMovementComponent->IsFalling());

	if (bCanJump)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			if (JumpCurrentCount == 0 && GravityMovementComponent->IsFalling())
			{
				bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bCanJump = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bCanJump = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return bCanJump;
}

void ALifeCharacter::ResetJumpState()
{
	bPressedJump = false;
	bWasJumping = false;
	JumpKeyHoldTime = 0.0f;
	JumpForceTimeRemaining = 0.0f;

	if (GravityMovementComponent && !GravityMovementComponent->IsFalling())
	{
		JumpCurrentCount = 0;
	}
}

void ALifeCharacter::OnJumped_Implementation()
{
}

bool ALifeCharacter::IsJumpProvidingForce() const
{
	if (JumpForceTimeRemaining > 0.0f)
	{
		return true;
	}
	else if (bProxyIsJumpForceApplied && (Role == ROLE_SimulatedProxy))
	{
		return GetWorld()->TimeSince(ProxyJumpForceStartedTime) <= GetJumpMaxHoldTime();
	}

	return false;
}

void ALifeCharacter::RecalculateBaseEyeHeight()
{
	if (!bIsCrouched)
	{
		Super::RecalculateBaseEyeHeight();
	}
	else
	{
		BaseEyeHeight = CrouchedEyeHeight;
	}
}


void ALifeCharacter::OnRep_IsCrouched()
{
	if (GravityMovementComponent)
	{
		if (bIsCrouched)
		{
			GravityMovementComponent->Crouch(true);
		}
		else
		{
			GravityMovementComponent->UnCrouch(true);
		}
	}
}

void ALifeCharacter::SetReplicateMovement(bool bInReplicateMovement)
{
	Super::SetReplicateMovement(bInReplicateMovement);

	if (GravityMovementComponent != nullptr && Role == ROLE_Authority)
	{
		// Set prediction data time stamp to current time to stop extrapolating
		// from time bReplicateMovement was turned off to when it was turned on again
		FNetworkPredictionData_Server* NetworkPrediction = GravityMovementComponent->HasPredictionData_Server() ? GravityMovementComponent->GetPredictionData_Server() : nullptr;

		if (NetworkPrediction != nullptr)
		{
			NetworkPrediction->ServerTimeStamp = GetWorld()->GetTimeSeconds();
		}
	}
}

bool ALifeCharacter::CanCrouch()
{
	return !bIsCrouched && GravityMovementComponent && GravityMovementComponent->CanEverCrouch() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void ALifeCharacter::Crouch(bool bClientSimulation)
{
	if (GravityMovementComponent)
	{
		if (CanCrouch())
		{
			GravityMovementComponent->bWantsToCrouch = true;
		}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		else if (!GravityMovementComponent->CanEverCrouch())
		{
			UE_LOG(LogCharacter, Log, TEXT("%s is trying to crouch, but crouching is disabled on this character! (check MovementComponent NavAgentSettings)"), *GetName());
		}
#endif
	}
}

void ALifeCharacter::UnCrouch(bool bClientSimulation)
{
	if (GravityMovementComponent)
	{
		GravityMovementComponent->bWantsToCrouch = false;
	}
}


void ALifeCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ALifeCharacter* DefaultChar = GetDefault<ALifeCharacter>(GetClass());
	if (Mesh && DefaultChar->Mesh)
	{
		Mesh->RelativeLocation.Z = DefaultChar->Mesh->RelativeLocation.Z;
		BaseTranslationOffset.Z = Mesh->RelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z;
	}

	K2_OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
}

void ALifeCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ALifeCharacter* DefaultChar = GetDefault<ALifeCharacter>(GetClass());
	if (Mesh && DefaultChar->Mesh)
	{
		Mesh->RelativeLocation.Z = DefaultChar->Mesh->RelativeLocation.Z + HeightAdjust;
		BaseTranslationOffset.Z = Mesh->RelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->BaseTranslationOffset.Z + HeightAdjust;
	}

	K2_OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
}

void ALifeCharacter::ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser)
{
	UDamageType const* const DmgTypeCDO = DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>();
	float const ImpulseScale = DmgTypeCDO->DamageImpulse;

	if ((ImpulseScale > 3.f) && (GravityMovementComponent != nullptr))
	{
		FHitResult HitInfo;
		FVector ImpulseDir;
		DamageEvent.GetBestHitInfo(this, PawnInstigator, HitInfo, ImpulseDir);

		FVector Impulse = ImpulseDir * ImpulseScale;
		bool const bMassIndependentImpulse = !DmgTypeCDO->bScaleMomentumByMass;

		// limit Z momentum added if already going up faster than jump (to avoid blowing character way up into the sky)
		{
			FVector MassScaledImpulse = Impulse;
			if (!bMassIndependentImpulse && GravityMovementComponent->Mass > SMALL_NUMBER)
			{
				MassScaledImpulse = MassScaledImpulse / GravityMovementComponent->Mass;
			}

			if ((GravityMovementComponent->Velocity.Z > GetDefault<UGravityMovementComponent>(GravityMovementComponent->GetClass())->JumpZVelocity) && (MassScaledImpulse.Z > 0.f))
			{
				Impulse.Z *= 0.5f;
			}
		}

		GravityMovementComponent->AddImpulse(Impulse, bMassIndependentImpulse);
	}
}

void ALifeCharacter::ClearCrossLevelReferences()
{
	if (BasedMovement.MovementBase != nullptr && GetOutermost() != BasedMovement.MovementBase->GetOutermost())
	{
		SetBase(nullptr);
	}

	Super::ClearCrossLevelReferences();
}


/**	Change the Pawn's base. */
void ALifeCharacter::SetBase(UPrimitiveComponent* NewBaseComponent, const FName InBoneName, bool bNotifyPawn)
{
	// If NewBaseComponent is nullptr, ignore bone name.
	const FName BoneName = (NewBaseComponent ? InBoneName : NAME_None);

	// See what changed.
	const bool bBaseChanged = (NewBaseComponent != BasedMovement.MovementBase);
	const bool bBoneChanged = (BoneName != BasedMovement.BoneName);

	if (bBaseChanged || bBoneChanged)
	{
		// Verify no recursion.
		APawn* Loop = (NewBaseComponent ? Cast<APawn>(NewBaseComponent->GetOwner()) : nullptr);
		while (Loop)
		{
			if (Loop == this)
			{
				UE_LOG(LogCharacter, Warning, TEXT(" SetBase failed! Recursion detected. Pawn %s already based on %s."), *GetName(), *NewBaseComponent->GetName()); //-V595
				return;
			}
			if (UPrimitiveComponent* LoopBase = Loop->GetMovementBase())
			{
				Loop = Cast<APawn>(LoopBase->GetOwner());
			}
			else
			{
				break;
			}
		}

		// Set base.
		UPrimitiveComponent* OldBase = BasedMovement.MovementBase;
		BasedMovement.MovementBase = NewBaseComponent;
		BasedMovement.BoneName = BoneName;

		if (GravityMovementComponent)
		{
			const bool bBaseIsSimulating = NewBaseComponent && NewBaseComponent->IsSimulatingPhysics();
			if (bBaseChanged)
			{
				MovementBaseUtility::RemoveTickDependency(GravityMovementComponent->PrimaryComponentTick, OldBase);
				// We use a special post physics function if simulating, otherwise add normal tick prereqs.
				if (!bBaseIsSimulating)
				{
					MovementBaseUtility::AddTickDependency(GravityMovementComponent->PrimaryComponentTick, NewBaseComponent);
				}
			}

			if (NewBaseComponent)
			{
				// Update OldBaseLocation/Rotation as those were referring to a different base
				// ... but not when handling replication for proxies (since they are going to copy this data from the replicated values anyway)
				if (!bInBaseReplication)
				{
					// Force base location and relative position to be computed since we have a new base or bone so the old relative offset is meaningless.
					GravityMovementComponent->SaveBaseLocation();
				}

				// Enable PostPhysics tick if we are standing on a physics object, as we need to to use post-physics transforms
				GravityMovementComponent->PostPhysicsTickFunction.SetTickFunctionEnable(bBaseIsSimulating);
			}
			else
			{
				BasedMovement.BoneName = NAME_None; // None, regardless of whether user tried to set a bone name, since we have no base component.
				BasedMovement.bRelativeRotation = false;
				GravityMovementComponent->CurrentFloor.Clear();
				GravityMovementComponent->PostPhysicsTickFunction.SetTickFunctionEnable(false);
			}

			if (Role == ROLE_Authority || Role == ROLE_AutonomousProxy)
			{
				BasedMovement.bServerHasBaseComponent = (BasedMovement.MovementBase != nullptr); // Also set on proxies for nicer debugging.
				UE_LOG(LogCharacter, Verbose, TEXT("Setting base on %s for '%s' to '%s'"), Role == ROLE_Authority ? TEXT("Server") : TEXT("AutoProxy"), *GetName(), *GetFullNameSafe(NewBaseComponent));
			}
			else
			{
				UE_LOG(LogCharacter, Verbose, TEXT("Setting base on Client for '%s' to '%s'"), *GetName(), *GetFullNameSafe(NewBaseComponent));
			}

		}

		// Notify this actor of his new floor.
		if (bNotifyPawn)
		{
			BaseChange();
		}
	}
}


void ALifeCharacter::SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation)
{
	checkSlow(BasedMovement.HasRelativeLocation());
	BasedMovement.Location = NewRelativeLocation;
	BasedMovement.Rotation = NewRotation;
	BasedMovement.bRelativeRotation = bRelativeRotation;
}

FVector ALifeCharacter::GetNavAgentLocation() const
{
	FVector AgentLocation = FNavigationSystem::InvalidLocation;

	if (GetGravityMovementComponent() != nullptr)
	{
		AgentLocation = GetGravityMovementComponent()->GetActorFeetLocation();
	}

	if (FNavigationSystem::IsValidLocation(AgentLocation) == false && CapsuleComponent != nullptr)
	{
		AgentLocation = GetActorLocation() - FVector(0, 0, CapsuleComponent->GetScaledCapsuleHalfHeight());
	}

	return AgentLocation;
}

void ALifeCharacter::TurnOff()
{
	if (GravityMovementComponent != nullptr)
	{
		GravityMovementComponent->StopMovementImmediately();
		GravityMovementComponent->DisableMovement();
	}

	if (GetNetMode() != NM_DedicatedServer && Mesh != nullptr)
	{
		Mesh->bPauseAnims = true;
		if (Mesh->IsSimulatingPhysics())
		{
			Mesh->bBlendPhysics = true;
			Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipAllBones;
		}
	}

	Super::TurnOff();
}

void ALifeCharacter::Restart()
{
	Super::Restart();

	JumpCurrentCount = 0;

	bPressedJump = false;
	ResetJumpState();
	UnCrouch(true);

	if (GravityMovementComponent)
	{
		GravityMovementComponent->SetDefaultMovementMode();
	}
}

void ALifeCharacter::PawnClientRestart()
{
	if (GravityMovementComponent != nullptr)
	{
		GravityMovementComponent->StopMovementImmediately();
		GravityMovementComponent->ResetPredictionData_Client();
	}

	Super::PawnClientRestart();
}

void ALifeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// If we are controlled remotely, set animation timing to be driven by client's network updates. So timing and events remain in sync.
	if (Mesh && bReplicateMovement && (GetRemoteRole() == ROLE_AutonomousProxy && GetNetConnection() != nullptr))
	{
		Mesh->bOnlyAllowAutonomousTickPose = true;
	}
}

void ALifeCharacter::UnPossessed()
{
	Super::UnPossessed();

	if (GravityMovementComponent)
	{
		GravityMovementComponent->ResetPredictionData_Client();
		GravityMovementComponent->ResetPredictionData_Server();
	}

	// We're no longer controlled remotely, resume regular ticking of animations.
	if (Mesh)
	{
		Mesh->bOnlyAllowAutonomousTickPose = false;
	}
}


void ALifeCharacter::TornOff()
{
	Super::TornOff();

	if (GravityMovementComponent)
	{
		GravityMovementComponent->ResetPredictionData_Client();
		GravityMovementComponent->ResetPredictionData_Server();
	}

	// We're no longer controlled remotely, resume regular ticking of animations.
	if (Mesh)
	{
		Mesh->bOnlyAllowAutonomousTickPose = false;
	}
}


void ALifeCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	NumActorOverlapEventsCounter++;
	Super::NotifyActorBeginOverlap(OtherActor);
}

void ALifeCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	NumActorOverlapEventsCounter++;
	Super::NotifyActorEndOverlap(OtherActor);
}

void ALifeCharacter::BaseChange()
{
	if (GravityMovementComponent && GravityMovementComponent->MovementMode != MOVE_None)
	{
		AActor* ActualMovementBase = GetMovementBaseActor(this);
		if ((ActualMovementBase != nullptr) && !ActualMovementBase->CanBeBaseForCharacter(this))
		{
			GravityMovementComponent->JumpOff(ActualMovementBase);
		}
	}
}

void ALifeCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	float Indent = 0.f;

	static FName NAME_Physics = FName(TEXT("Physics"));
	if (DebugDisplay.IsDisplayOn(NAME_Physics))
	{
		FIndenter PhysicsIndent(Indent);

		FString BaseString;
		if (GravityMovementComponent == nullptr || BasedMovement.MovementBase == nullptr)
		{
			BaseString = "Not Based";
		}
		else
		{
			BaseString = BasedMovement.MovementBase->IsWorldGeometry() ? "World Geometry" : BasedMovement.MovementBase->GetName();
			BaseString = FString::Printf(TEXT("Based On %s"), *BaseString);
		}

		FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
		DisplayDebugManager.DrawString(FString::Printf(TEXT("RelativeLoc: %s Rot: %s %s"), *BasedMovement.Location.ToCompactString(), *BasedMovement.Rotation.ToCompactString(), *BaseString), Indent);

		if (GravityMovementComponent != nullptr)
		{
			GravityMovementComponent->DisplayDebug(Canvas, DebugDisplay, YL, YPos);
		}
		const bool Crouched = GravityMovementComponent && GravityMovementComponent->IsCrouching();
		FString T = FString::Printf(TEXT("Crouched %i"), Crouched);
		DisplayDebugManager.DrawString(T, Indent);
	}
}

void ALifeCharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	UE_LOG(LogCharacter, Verbose, TEXT("ALifeCharacter::LaunchCharacter '%s' (%f,%f,%f)"), *GetName(), LaunchVelocity.X, LaunchVelocity.Y, LaunchVelocity.Z);

	if (GravityMovementComponent)
	{
		FVector FinalVel = LaunchVelocity;
		const FVector Velocity = GetVelocity();

		if (!bXYOverride)
		{
			FinalVel.X += Velocity.X;
			FinalVel.Y += Velocity.Y;
		}
		if (!bZOverride)
		{
			FinalVel.Z += Velocity.Z;
		}

		GravityMovementComponent->Launch(FinalVel);

		OnLaunched(LaunchVelocity, bXYOverride, bZOverride);
	}
}


void ALifeCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	if (!bPressedJump || !GravityMovementComponent->IsFalling())
	{
		ResetJumpState();
	}

	// Recored jump force start time for proxies. Allows us to expire the jump even if not continually ticking down a timer.
	if (bProxyIsJumpForceApplied && GravityMovementComponent->IsFalling())
	{
		ProxyJumpForceStartedTime = GetWorld()->GetTimeSeconds();
	}

	K2_OnMovementModeChanged(PrevMovementMode, GravityMovementComponent->MovementMode, PrevCustomMode, GravityMovementComponent->CustomMovementMode);
	MovementModeChangedDelegate.Broadcast(this, PrevMovementMode, PrevCustomMode);
}


/** Don't process landed notification if updating client position by replaying moves.
 * Allow event to be called if Pawn was initially falling (before starting to replay moves),
 * and this is going to cause him to land. . */
bool ALifeCharacter::ShouldNotifyLanded(const FHitResult& Hit)
{
	if (bClientUpdating && !bClientWasFalling)
	{
		return false;
	}

	// Just in case, only allow Landed() to be called once when replaying moves.
	bClientWasFalling = false;
	return true;
}

void ALifeCharacter::Jump()
{
	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;
}

void ALifeCharacter::StopJumping()
{
	bPressedJump = false;
	ResetJumpState();
}

void ALifeCharacter::CheckJumpInput(float DeltaTime)
{
	if (GravityMovementComponent)
	{
		if (bPressedJump)
		{
			// If this is the first jump and we're already falling,
			// then increment the JumpCount to compensate.
			const bool bFirstJump = JumpCurrentCount == 0;
			if (bFirstJump && GravityMovementComponent->IsFalling())
			{
				JumpCurrentCount++;
			}

			const bool bDidJump = CanJump() && GravityMovementComponent->DoJump(bClientUpdating);
			if (bDidJump)
			{
				// Transition from not (actively) jumping to jumping.
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();
					OnJumped();
				}
			}

			bWasJumping = bDidJump;
		}
	}
}


void ALifeCharacter::ClearJumpInput(float DeltaTime)
{
	if (bPressedJump)
	{
		JumpKeyHoldTime += DeltaTime;

		// Don't disable bPressedJump right away if it's still held.
		// Don't modify JumpForceTimeRemaining because a frame of update may be remaining.
		if (JumpKeyHoldTime >= GetJumpMaxHoldTime())
		{
			bPressedJump = false;
		}
	}
	else
	{
		JumpForceTimeRemaining = 0.0f;
		bWasJumping = false;
	}
}

float ALifeCharacter::GetJumpMaxHoldTime() const
{
	return JumpMaxHoldTime;
}

//
// Static variables for networking.
//
static uint8 SavedMovementMode;

void ALifeCharacter::PreNetReceive()
{
	SavedMovementMode = ReplicatedMovementMode;
	Super::PreNetReceive();
}

void ALifeCharacter::PostNetReceive()
{
	if (Role == ROLE_SimulatedProxy)
	{
		GravityMovementComponent->bNetworkUpdateReceived = true;
		GravityMovementComponent->bNetworkMovementModeChanged = (GravityMovementComponent->bNetworkMovementModeChanged || (SavedMovementMode != ReplicatedMovementMode));
	}

	Super::PostNetReceive();
}

void ALifeCharacter::OnRep_ReplicatedBasedMovement()
{
	if (Role != ROLE_SimulatedProxy)
	{
		return;
	}

	// Skip base updates while playing root motion, it is handled inside of OnRep_RootMotion
	if (IsPlayingNetworkedRootMotionMontage())
	{
		return;
	}

	TGuardValue<bool> bInBaseReplicationGuard(bInBaseReplication, true);

	const bool bBaseChanged = (BasedMovement.MovementBase != ReplicatedBasedMovement.MovementBase || BasedMovement.BoneName != ReplicatedBasedMovement.BoneName);
	if (bBaseChanged)
	{
		// Even though we will copy the replicated based movement info, we need to use SetBase() to set up tick dependencies and trigger notifications.
		SetBase(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName);
	}

	// Make sure to use the values of relative location/rotation etc from the server.
	BasedMovement = ReplicatedBasedMovement;

	if (ReplicatedBasedMovement.HasRelativeLocation())
	{
		// Update transform relative to movement base
		const FVector OldLocation = GetActorLocation();
		const FQuat OldRotation = GetActorQuat();
		MovementBaseUtility::GetMovementBaseTransform(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName, GravityMovementComponent->OldBaseLocation, GravityMovementComponent->OldBaseQuat);
		const FVector NewLocation = GravityMovementComponent->OldBaseLocation + ReplicatedBasedMovement.Location;
		FRotator NewRotation;

		if (ReplicatedBasedMovement.HasRelativeRotation())
		{
			// Relative location, relative rotation
			NewRotation = (FRotationMatrix(ReplicatedBasedMovement.Rotation) * FQuatRotationMatrix(GravityMovementComponent->OldBaseQuat)).Rotator();

			if (GravityMovementComponent->ShouldRemainVertical())
			{
				NewRotation.Pitch = 0.f;
				NewRotation.Roll = 0.f;
			}
		}
		else
		{
			// Relative location, absolute rotation
			NewRotation = ReplicatedBasedMovement.Rotation;
		}

		// When position or base changes, movement mode will need to be updated. This assumes rotation changes don't affect that.
		GravityMovementComponent->bJustTeleported |= (bBaseChanged || NewLocation != OldLocation);
		GravityMovementComponent->bNetworkSmoothingComplete = false;
		GravityMovementComponent->SmoothCorrection(OldLocation, OldRotation, NewLocation, NewRotation.Quaternion());
		OnUpdateSimulatedPosition(OldLocation, OldRotation);
	}
}

void ALifeCharacter::OnRep_ReplicatedMovement()
{
	// Skip standard position correction if we are playing root motion, OnRep_RootMotion will handle it.
	if (!IsPlayingNetworkedRootMotionMontage()) // animation root motion
	{
		if (!GravityMovementComponent || !GravityMovementComponent->CurrentRootMotion.HasActiveRootMotionSources()) // root motion sources
		{
			Super::OnRep_ReplicatedMovement();
		}
	}
}

/** Get FAnimMontageInstance playing RootMotion */
FAnimMontageInstance * ALifeCharacter::GetRootMotionAnimMontageInstance() const
{
	return (Mesh && Mesh->GetAnimInstance()) ? Mesh->GetAnimInstance()->GetRootMotionMontageInstance() : nullptr;
}

void ALifeCharacter::OnRep_RootMotion()
{
	if (Role == ROLE_SimulatedProxy)
	{
		UE_LOG(LogRootMotion, Log, TEXT("ALifeCharacter::OnRep_RootMotion"));

		// Save received move in queue, we'll try to use it during Tick().
		if (RepRootMotion.bIsActive)
		{
			if (GravityMovementComponent)
			{
				// Add new move
				RootMotionRepMoves.AddZeroed(1);
				FSimulatedRootMotionReplicatedMove& NewMove = RootMotionRepMoves.Last();
				NewMove.RootMotion = RepRootMotion;
				NewMove.Time = GetWorld()->GetTimeSeconds();

				// Convert RootMotionSource Server IDs -> Local IDs in AuthoritativeRootMotion and cull invalid
				// so that when we use this root motion it has the correct IDs
				GravityMovementComponent->ConvertRootMotionServerIDsToLocalIDs(GravityMovementComponent->CurrentRootMotion, NewMove.RootMotion.AuthoritativeRootMotion, NewMove.Time);
				NewMove.RootMotion.AuthoritativeRootMotion.CullInvalidSources();
			}
		}
		else
		{
			// Clear saved moves.
			RootMotionRepMoves.Empty();
		}
	}
}

void ALifeCharacter::SimulatedRootMotionPositionFixup(float DeltaSeconds)
{
	const FAnimMontageInstance* ClientMontageInstance = GetRootMotionAnimMontageInstance();
	if (ClientMontageInstance && GravityMovementComponent && Mesh)
	{
		// Find most recent buffered move that we can use.
		const int32 MoveIndex = FindRootMotionRepMove(*ClientMontageInstance);
		if (MoveIndex != INDEX_NONE)
		{
			const FVector OldLocation = GetActorLocation();
			const FQuat OldRotation = GetActorQuat();
			// Move Actor back to position of that buffered move. (server replicated position).
			const FSimulatedRootMotionReplicatedMove& RootMotionRepMove = RootMotionRepMoves[MoveIndex];
			if (RestoreReplicatedMove(RootMotionRepMove))
			{
				const float ServerPosition = RootMotionRepMove.RootMotion.Position;
				const float ClientPosition = ClientMontageInstance->GetPosition();
				const float DeltaPosition = (ClientPosition - ServerPosition);
				if (FMath::Abs(DeltaPosition) > KINDA_SMALL_NUMBER)
				{
					// Find Root Motion delta move to get back to where we were on the client.
					const FTransform LocalRootMotionTransform = ClientMontageInstance->Montage->ExtractRootMotionFromTrackRange(ServerPosition, ClientPosition);

					// Simulate Root Motion for delta move.
					if (GravityMovementComponent)
					{
						const float MontagePlayRate = ClientMontageInstance->GetPlayRate();
						// Guess time it takes for this delta track position, so we can get falling physics accurate.
						if (!FMath::IsNearlyZero(MontagePlayRate))
						{
							const float DeltaTime = DeltaPosition / MontagePlayRate;

							// Even with negative playrate deltatime should be positive.
							check(DeltaTime > 0.f);
							GravityMovementComponent->SimulateRootMotion(DeltaTime, LocalRootMotionTransform);

							// After movement correction, smooth out error in position if any.
							GravityMovementComponent->bNetworkSmoothingComplete = false;
							GravityMovementComponent->SmoothCorrection(OldLocation, OldRotation, GetActorLocation(), GetActorQuat());
						}
					}
				}
			}

			// Delete this move and any prior one, we don't need them anymore.
			UE_LOG(LogRootMotion, Log, TEXT("\tClearing old moves (%d)"), MoveIndex + 1);
			RootMotionRepMoves.RemoveAt(0, MoveIndex + 1);
		}
	}
}

int32 ALifeCharacter::FindRootMotionRepMove(const FAnimMontageInstance& ClientMontageInstance) const
{
	int32 FoundIndex = INDEX_NONE;

	// Start with most recent move and go back in time to find a usable move.
	for (int32 MoveIndex = RootMotionRepMoves.Num() - 1; MoveIndex >= 0; MoveIndex--)
	{
		if (CanUseRootMotionRepMove(RootMotionRepMoves[MoveIndex], ClientMontageInstance))
		{
			FoundIndex = MoveIndex;
			break;
		}
	}

	UE_LOG(LogRootMotion, Log, TEXT("\tALifeCharacter::FindRootMotionRepMove FoundIndex: %d, NumSavedMoves: %d"), FoundIndex, RootMotionRepMoves.Num());
	return FoundIndex;
}

bool ALifeCharacter::CanUseRootMotionRepMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove, const FAnimMontageInstance& ClientMontageInstance) const
{
	// Ignore outdated moves.
	if (GetWorld()->TimeSince(RootMotionRepMove.Time) <= 0.5f)
	{
		// Make sure montage being played matched between client and server.
		if (RootMotionRepMove.RootMotion.AnimMontage && (RootMotionRepMove.RootMotion.AnimMontage == ClientMontageInstance.Montage))
		{
			UAnimMontage * AnimMontage = ClientMontageInstance.Montage;
			const float ServerPosition = RootMotionRepMove.RootMotion.Position;
			const float ClientPosition = ClientMontageInstance.GetPosition();
			const float DeltaPosition = (ClientPosition - ServerPosition);
			const int32 CurrentSectionIndex = AnimMontage->GetSectionIndexFromPosition(ClientPosition);
			if (CurrentSectionIndex != INDEX_NONE)
			{
				const int32 NextSectionIndex = ClientMontageInstance.GetNextSectionID(CurrentSectionIndex);

				// We can only extract root motion if we are within the same section.
				// It's not trivial to jump through sections in a deterministic manner, but that is luckily not frequent. 
				const bool bSameSections = (AnimMontage->GetSectionIndexFromPosition(ServerPosition) == CurrentSectionIndex);
				// if we are looping and just wrapped over, skip. That's also not easy to handle and not frequent.
				const bool bHasLooped = (NextSectionIndex == CurrentSectionIndex) && (FMath::Abs(DeltaPosition) > (AnimMontage->GetSectionLength(CurrentSectionIndex) / 2.f));
				// Can only simulate forward in time, so we need to make sure server move is not ahead of the client.
				const bool bServerAheadOfClient = ((DeltaPosition * ClientMontageInstance.GetPlayRate()) < 0.f);

				UE_LOG(LogRootMotion, Log, TEXT("\t\tALifeCharacter::CanUseRootMotionRepMove ServerPosition: %.3f, ClientPosition: %.3f, DeltaPosition: %.3f, bSameSections: %d, bHasLooped: %d, bServerAheadOfClient: %d"),
					ServerPosition, ClientPosition, DeltaPosition, bSameSections, bHasLooped, bServerAheadOfClient);

				return bSameSections && !bHasLooped && !bServerAheadOfClient;
			}
		}
	}
	return false;
}

bool ALifeCharacter::RestoreReplicatedMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove)
{
	UPrimitiveComponent* ServerBase = RootMotionRepMove.RootMotion.MovementBase;
	const FName ServerBaseBoneName = RootMotionRepMove.RootMotion.MovementBaseBoneName;

	// Relative Position
	if (RootMotionRepMove.RootMotion.bRelativePosition)
	{
		bool bSuccess = false;
		if (MovementBaseUtility::UseRelativeLocation(ServerBase))
		{
			FVector BaseLocation;
			FQuat BaseRotation;
			MovementBaseUtility::GetMovementBaseTransform(ServerBase, ServerBaseBoneName, BaseLocation, BaseRotation);

			const FVector ServerLocation = BaseLocation + RootMotionRepMove.RootMotion.Location;
			FRotator ServerRotation;
			if (RootMotionRepMove.RootMotion.bRelativeRotation)
			{
				// Relative rotation
				ServerRotation = (FRotationMatrix(RootMotionRepMove.RootMotion.Rotation) * FQuatRotationTranslationMatrix(BaseRotation, FVector::ZeroVector)).Rotator();
			}
			else
			{
				// Absolute rotation
				ServerRotation = RootMotionRepMove.RootMotion.Rotation;
			}

			SetActorLocationAndRotation(ServerLocation, ServerRotation);
			bSuccess = true;
		}
		// If we received local space position, but can't resolve parent, then move can't be used. :(
		if (!bSuccess)
		{
			return false;
		}
	}
	// Absolute position
	else
	{
		FVector LocalLocation = FRepMovement::RebaseOntoLocalOrigin(RootMotionRepMove.RootMotion.Location, this);
		SetActorLocationAndRotation(LocalLocation, RootMotionRepMove.RootMotion.Rotation);
	}

	GravityMovementComponent->bJustTeleported = true;
	SetBase(ServerBase, ServerBaseBoneName);

	return true;
}

void ALifeCharacter::OnUpdateSimulatedPosition(const FVector& OldLocation, const FQuat& OldRotation)
{
	SCOPE_CYCLE_COUNTER(STAT_CharacterOnNetUpdateSimulatedPosition);

	bSimGravityDisabled = false;
	const bool bLocationChanged = (OldLocation != GetActorLocation());
	if (bClientCheckEncroachmentOnNetUpdate)
	{
		// Only need to check for encroachment when teleported without any velocity.
		// Normal movement pops the character out of geometry anyway, no use doing it before and after (with different rules).
		// Always consider Location as changed if we were spawned this tick as in that case our replicated Location was set as part of spawning, before PreNetReceive()
		if (GravityMovementComponent->Velocity.IsZero() && (bLocationChanged || CreationTime == GetWorld()->TimeSeconds))
		{
			if (GetWorld()->EncroachingBlockingGeometry(this, GetActorLocation(), GetActorRotation()))
			{
				bSimGravityDisabled = true;
			}
		}
	}
	GravityMovementComponent->bJustTeleported |= bLocationChanged;
}

void ALifeCharacter::PostNetReceiveLocationAndRotation()
{
	if (Role == ROLE_SimulatedProxy)
	{
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation())
		{
			const FVector OldLocation = GetActorLocation();
			const FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(ReplicatedMovement.Location, this);
			const FQuat OldRotation = GetActorQuat();

			GravityMovementComponent->bNetworkSmoothingComplete = false;
			GravityMovementComponent->SmoothCorrection(OldLocation, OldRotation, NewLocation, ReplicatedMovement.Rotation.Quaternion());
			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
	}
}

void ALifeCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (GravityMovementComponent->CurrentRootMotion.HasActiveRootMotionSources() || IsPlayingNetworkedRootMotionMontage())
	{
		const FAnimMontageInstance* RootMotionMontageInstance = GetRootMotionAnimMontageInstance();

		RepRootMotion.bIsActive = true;
		// Is position stored in local space?
		RepRootMotion.bRelativePosition = BasedMovement.HasRelativeLocation();
		RepRootMotion.bRelativeRotation = BasedMovement.HasRelativeRotation();
		RepRootMotion.Location = RepRootMotion.bRelativePosition ? BasedMovement.Location : FRepMovement::RebaseOntoZeroOrigin(GetActorLocation(), GetWorld()->OriginLocation);
		RepRootMotion.Rotation = RepRootMotion.bRelativeRotation ? BasedMovement.Rotation : GetActorRotation();
		RepRootMotion.MovementBase = BasedMovement.MovementBase;
		RepRootMotion.MovementBaseBoneName = BasedMovement.BoneName;
		if (RootMotionMontageInstance)
		{
			RepRootMotion.AnimMontage = RootMotionMontageInstance->Montage;
			RepRootMotion.Position = RootMotionMontageInstance->GetPosition();
		}
		else
		{
			RepRootMotion.AnimMontage = nullptr;
		}

		RepRootMotion.AuthoritativeRootMotion = GravityMovementComponent->CurrentRootMotion;
		RepRootMotion.Acceleration = GravityMovementComponent->GetCurrentAcceleration();
		RepRootMotion.LinearVelocity = GravityMovementComponent->Velocity;

		DOREPLIFETIME_ACTIVE_OVERRIDE(ALifeCharacter, RepRootMotion, true);
	}
	else
	{
		RepRootMotion.Clear();

		DOREPLIFETIME_ACTIVE_OVERRIDE(ALifeCharacter, RepRootMotion, false);
	}

	bProxyIsJumpForceApplied = (JumpForceTimeRemaining > 0.0f);
	ReplicatedServerLastTransformUpdateTimeStamp = GravityMovementComponent->GetServerLastTransformUpdateTimeStamp();
	ReplicatedMovementMode = GravityMovementComponent->PackNetworkMovementMode();
	ReplicatedBasedMovement = BasedMovement;

	// Optimization: only update and replicate these values if they are actually going to be used.
	if (BasedMovement.HasRelativeLocation())
	{
		// When velocity becomes zero, force replication so the position is updated to match the server (it may have moved due to simulation on the client).
		ReplicatedBasedMovement.bServerHasVelocity = !GravityMovementComponent->Velocity.IsZero();

		// Make sure absolute rotations are updated in case rotation occurred after the base info was saved.
		if (!BasedMovement.HasRelativeRotation())
		{
			ReplicatedBasedMovement.Rotation = GetActorRotation();
		}
	}

	// Save bandwidth by not replicating this value unless it is necessary, since it changes every update.
	if ((GravityMovementComponent->NetworkSmoothingMode != ENetworkSmoothingMode::Linear) && !GravityMovementComponent->bNetworkAlwaysReplicateTransformUpdateTimestamp)
	{
		ReplicatedServerLastTransformUpdateTimeStamp = 0.f;
	}
}

void ALifeCharacter::PreReplicationForReplay(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplicationForReplay(ChangedPropertyTracker);

	// If this is a replay, we save out certain values we need to runtime to do smooth interpolation
	// We'll be able to look ahead in the replay to have these ahead of time for smoother playback
	FCharacterReplaySample ReplaySample;

	const UWorld* World = GetWorld();

	// If this is a client-recorded replay, use the mesh location and rotation, since these will always
	// be smoothed - unlike the actor position and rotation.
	const USkeletalMeshComponent* const MeshComponent = GetMesh();
	if (MeshComponent && World && World->IsRecordingClientReplay())
	{
		FNetworkPredictionData_Client_Character const* const ClientNetworkPredicationData = GravityMovementComponent->GetPredictionData_Client_Character();
		if ((Role == ROLE_SimulatedProxy) && ClientNetworkPredicationData)
		{
			ReplaySample.Location = GetActorLocation() + ClientNetworkPredicationData->MeshRotationOffset.UnrotateVector(ClientNetworkPredicationData->MeshTranslationOffset);
			ReplaySample.Rotation = GetActorRotation() + ClientNetworkPredicationData->MeshRotationOffset.Rotator();
		}
		else
		{
			// Remove the base transform from the mesh's transform, since on playback the base transform
			// will be stored in the mesh's RelativeLocation and RelativeRotation.
			const FTransform BaseTransform(GetBaseRotationOffset(), GetBaseTranslationOffset());
			const FTransform MeshRootTransform = BaseTransform.Inverse() * MeshComponent->GetComponentTransform();

			ReplaySample.Location = MeshRootTransform.GetLocation();
			ReplaySample.Rotation = MeshRootTransform.GetRotation().Rotator();
		}

		// On client replays, our view pitch will be set to 0 as by default we do not replicate
		// pitch for owners, just for simulated. So instead push our rotation into the sampler
		if (Controller != nullptr && Role == ROLE_AutonomousProxy && GetNetMode() == NM_Client)
		{
			SetRemoteViewPitch(Controller->GetControlRotation().Pitch);
		}
	}
	else
	{
		ReplaySample.Location = GetActorLocation();
		ReplaySample.Rotation = GetActorRotation();
	}

	ReplaySample.Velocity = GetVelocity();
	ReplaySample.Acceleration = GravityMovementComponent->GetCurrentAcceleration();
	ReplaySample.RemoteViewPitch = RemoteViewPitch;

	if (World && World->DemoNetDriver)
	{
		ReplaySample.Time = World->DemoNetDriver->DemoCurrentTime;
	}

	FBitWriter Writer(0, true);
	Writer << ReplaySample;

	ChangedPropertyTracker.SetExternalData(Writer.GetData(), Writer.GetNumBits());
}


void ALifeCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALifeCharacter, RepRootMotion, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(ALifeCharacter, ReplicatedBasedMovement, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ALifeCharacter, ReplicatedServerLastTransformUpdateTimeStamp, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(ALifeCharacter, ReplicatedMovementMode, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ALifeCharacter, bIsCrouched, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ALifeCharacter, bProxyIsJumpForceApplied, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ALifeCharacter, AnimRootMotionTranslationScale, COND_SimulatedOnly);

	// Change the condition of the replicated movement property to not replicate in replays since we handle this specifically via saving this out in external replay data
	DOREPLIFETIME_CHANGE_CONDITION(AActor, ReplicatedMovement, COND_SimulatedOrPhysicsNoReplay);
}

bool ALifeCharacter::IsPlayingRootMotion() const
{
	if (Mesh)
	{
		return Mesh->IsPlayingRootMotion();
	}
	return false;
}

bool ALifeCharacter::IsPlayingNetworkedRootMotionMontage() const
{
	if (Mesh)
	{
		return Mesh->IsPlayingNetworkedRootMotionMontage();
	}
	return false;
}

void ALifeCharacter::SetAnimRootMotionTranslationScale(float InAnimRootMotionTranslationScale)
{
	AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
}

float ALifeCharacter::GetAnimRootMotionTranslationScale() const
{
	return AnimRootMotionTranslationScale;
}

float ALifeCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return 0.f;
}

void ALifeCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	UAnimMontage * MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentMontage();
	bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
	}
}

class UAnimMontage * ALifeCharacter::GetCurrentMontage()
{
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		return AnimInstance->GetCurrentActiveMontage();
	}

	return nullptr;
}

void ALifeCharacter::ClientCheatWalk_Implementation()
{
#if !UE_BUILD_SHIPPING
	SetActorEnableCollision(true);
	if (GravityMovementComponent)
	{
		GravityMovementComponent->bCheatFlying = false;
		GravityMovementComponent->SetMovementMode(MOVE_Falling);
	}
#endif
}

void ALifeCharacter::ClientCheatFly_Implementation()
{
#if !UE_BUILD_SHIPPING
	SetActorEnableCollision(true);
	if (GravityMovementComponent)
	{
		GravityMovementComponent->bCheatFlying = true;
		GravityMovementComponent->SetMovementMode(MOVE_Flying);
	}
#endif
}

void ALifeCharacter::ClientCheatGhost_Implementation()
{
#if !UE_BUILD_SHIPPING
	SetActorEnableCollision(false);
	if (GravityMovementComponent)
	{
		GravityMovementComponent->bCheatFlying = true;
		GravityMovementComponent->SetMovementMode(MOVE_Flying);
	}
#endif
}

void ALifeCharacter::RootMotionDebugClientPrintOnScreen_Implementation(const FString& InString)
{
#if ROOT_MOTION_DEBUG
	RootMotionSourceDebug::PrintOnScreenServerMsg(InString);
#endif
}


// ServerMove
void ALifeCharacter::ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	GetGravityMovementComponent()->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool ALifeCharacter::ServerMove_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	return GetGravityMovementComponent()->ServerMove_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveNoBase
void ALifeCharacter::ServerMoveNoBase_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode)
{
	GetGravityMovementComponent()->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

bool ALifeCharacter::ServerMoveNoBase_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode)
{
	return GetGravityMovementComponent()->ServerMove_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

// ServerMoveDual
void ALifeCharacter::ServerMoveDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	GetGravityMovementComponent()->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool ALifeCharacter::ServerMoveDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	return GetGravityMovementComponent()->ServerMoveDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveDualNoBase
void ALifeCharacter::ServerMoveDualNoBase_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode)
{
	GetGravityMovementComponent()->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

bool ALifeCharacter::ServerMoveDualNoBase_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode)
{
	return GetGravityMovementComponent()->ServerMoveDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

// ServerMoveDualHybridRootMotion
void ALifeCharacter::ServerMoveDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	GetGravityMovementComponent()->ServerMoveDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool ALifeCharacter::ServerMoveDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	return GetGravityMovementComponent()->ServerMoveDualHybridRootMotion_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveOld
void ALifeCharacter::ServerMoveOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags)
{
	GetGravityMovementComponent()->ServerMoveOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags);
}

bool ALifeCharacter::ServerMoveOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags)
{
	return GetGravityMovementComponent()->ServerMoveOld_Validate(OldTimeStamp, OldAccel, OldMoveFlags);
}

// ClientAckGoodMove
void ALifeCharacter::ClientAckGoodMove_Implementation(float TimeStamp)
{
	GetGravityMovementComponent()->ClientAckGoodMove_Implementation(TimeStamp);
}

// ClientAdjustPosition
void ALifeCharacter::ClientAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	GetGravityMovementComponent()->ClientAdjustPosition_Implementation(TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientVeryShortAdjustPosition
void ALifeCharacter::ClientVeryShortAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	GetGravityMovementComponent()->ClientVeryShortAdjustPosition_Implementation(TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientAdjustRootMotionPosition
void ALifeCharacter::ClientAdjustRootMotionPosition_Implementation(float TimeStamp, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	GetGravityMovementComponent()->ClientAdjustRootMotionPosition_Implementation(TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientAdjustRootMotionSourcePosition
void ALifeCharacter::ClientAdjustRootMotionSourcePosition_Implementation(float TimeStamp, FRootMotionSourceGroup ServerRootMotion, bool bHasAnimRootMotion, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	GetGravityMovementComponent()->ClientAdjustRootMotionSourcePosition_Implementation(TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}
