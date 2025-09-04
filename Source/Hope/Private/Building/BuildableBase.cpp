// Copyright Sertim all rights reserved


#include "Building/BuildableBase.h"

ABuildableBase::ABuildableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));
	SetRootComponent(BaseMeshComponent);
	bReplicates = true;
	SetReplicateMovement(true);
}

void ABuildableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

