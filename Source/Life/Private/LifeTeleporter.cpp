// Fill out your copyright notice in the Description page of Project Settings.

#include "Life.h"
#include "LifeCharacter.h"
#include "LifeTeleporter.h"



ALifeTeleporter::ALifeTeleporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UCapsuleComponent* CollisionComp = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("CollisionComp"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	TeleportDestinationComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("TeleportDestinationComponent"));
	if (TeleportDestinationComponent)
	{
		TeleportDestinationComponent->SetupAttachment(CollisionComp);
		TeleportDestinationComponent->AddRelativeLocation(CollisionComp->GetForwardVector() * 100.0f);
	}

	StaticMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh0"));
	if (StaticMesh)
	{
		StaticMesh->bCastDynamicShadow = true;
		StaticMesh->bAffectDynamicIndirectLighting = true;
		StaticMesh->SetGenerateOverlapEvents(false);
		StaticMesh->SetNotifyRigidBodyCollision(false);
		StaticMesh->SetupAttachment(CollisionComp);
	}

	TeleportPSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("TeleportPSC"));
	TeleportPSC->bAutoActivate = false;
	TeleportPSC->SetupAttachment(RootComponent);

	ActivePSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ActivePSC"));
	ActivePSC->bAutoActivate = true;
	ActivePSC->SetupAttachment(RootComponent);

	bCanTeleport = true;
	TeleportTime = 2.0f;
	CameraWaitTime = 2.0f;
	TeleportInactiveTime = 4.0f;
}

void ALifeTeleporter::BeginPlay()
{
	Super::BeginPlay();
	if (TeleporterActiveSound)
	{
		UGameplayStatics::SpawnSoundAttached(TeleporterActiveSound, GetRootComponent());
	}
}

void ALifeTeleporter::NotifyActorBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);
	ALifeCharacter* LifeCharacter = Cast<ALifeCharacter>(Other);
	if (LifeCharacter && bCanTeleport)
	{
		DoTeleport(LifeCharacter);
	}
}

void ALifeTeleporter::DoTeleport(ALifeCharacter* LifeCharacter)
{
	if (OtherTeleporter)
	{
		if (TeleportPSC)
		{
			TeleportPSC->ActivateSystem();
		}
		if (TeleportSound)
		{
			UGameplayStatics::SpawnSoundAttached(TeleportSound, GetRootComponent());
		}
		OtherTeleporter->ReceiveTeleport(LifeCharacter);
	}
}

void ALifeTeleporter::ReceiveTeleport(ALifeCharacter* LifeCharacter)
{
	if (TeleportDestinationComponent)
	{
		bCanTeleport = false;
		GetWorld()->GetTimerManager().SetTimer(CanTeleportHandle, this, &ALifeTeleporter::SetCanTeleport, TeleportInactiveTime, false);
		LifeCharacter->TeleportCharacter(this);
		if (TeleportPSC)
		{
			TeleportPSC->ActivateSystem();
		}
		if (TeleportSound)
		{
			UGameplayStatics::SpawnSoundAttached(TeleportSound, GetRootComponent());
		}
	}
}

void ALifeTeleporter::SetCanTeleport()
{
	bCanTeleport = true;
	GetWorld()->GetTimerManager().ClearTimer(CanTeleportHandle);
}