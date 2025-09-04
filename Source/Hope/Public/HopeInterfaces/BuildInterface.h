// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "BuildInterface.generated.h"

//class UBoxComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBuildInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HOPE_API IBuildInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Build Interface")
	TArray<UBoxComponent*> ReturnBoxes();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Build Interface")
	void InteractWithBuilding();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Build Interface")
	EBuildingType GetBuildingType();
};
