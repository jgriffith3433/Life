// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Life.h"
#include "LifePickup.h"
#include "LifeCharacter.h"
#include "LifeGameMode.h"

ALifePickup::ALifePickup(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UCapsuleComponent* CollisionComp = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("CollisionComp"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	StaticMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh0"));
	if (StaticMesh)
	{
		StaticMesh->bCastDynamicShadow = true;
		StaticMesh->bAffectDynamicIndirectLighting = true;
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMesh->SetGenerateOverlapEvents(false);
		StaticMesh->SetNotifyRigidBodyCollision(false);
		StaticMesh->SetupAttachment(CollisionComp);
	}

	PickupPSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("PickupPSC"));
	PickupPSC->bAutoActivate = false;
	PickupPSC->SetupAttachment(RootComponent);

	ActivePSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ActivePSC"));
	ActivePSC->bAutoActivate = true;
	ActivePSC->SetupAttachment(RootComponent);

	bCanPickup = true;
	TimeBeforeDestroy = 1.0f;
}

void ALifePickup::BeginPlay()
{
	Super::BeginPlay();
}

void ALifePickup::GivePickupTo(class ALifeCharacter* LifeCharacter)
{

}

void ALifePickup::NotifyActorBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);
	ALifeCharacter* LifeCharacter = Cast<ALifeCharacter>(Other);
	if (LifeCharacter && !IsPendingKill() && bCanPickup)
	{
		GivePickupTo(LifeCharacter);
		if (PickupPSC)
		{
			PickupPSC->ActivateSystem();
		}
		if (ActivePSC)
		{
			ActivePSC->DeactivateSystem();
		}
		if (PickupSound)
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, LifeCharacter->GetRootComponent());
		}
		bCanPickup = false;
		StaticMesh->SetVisibility(false);
		GetWorld()->GetTimerManager().SetTimer(DestroyPickupHandle, this, &ALifePickup::DestroyPickup, TimeBeforeDestroy, false);
	}
}

void ALifePickup::DestroyPickup()
{
	GetWorld()->GetTimerManager().ClearTimer(DestroyPickupHandle);
	Destroy();
}
