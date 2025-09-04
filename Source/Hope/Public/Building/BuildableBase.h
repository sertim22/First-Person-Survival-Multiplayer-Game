// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HopeInterfaces/BuildInterface.h"
#include "BuildableBase.generated.h"

UCLASS()
class HOPE_API ABuildableBase : public AActor, public IBuildInterface
{
	GENERATED_BODY()
	
public:	
	
	ABuildableBase();

	/*Build Interface*/

	virtual EBuildingType GetBuildingType_Implementation() override { return BuildingType; }

	/*Build Interface*/

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Building Properties")
	EBuildingType BuildingType;

protected:
	
	virtual void BeginPlay() override;
};
