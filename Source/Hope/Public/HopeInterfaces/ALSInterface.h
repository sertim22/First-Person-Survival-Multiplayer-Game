// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALS/ALSDataTypes.h"
#include "ALSInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UALSInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface used by Anim Instances of this project.
 */
class HOPE_API IALSInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALS Interface")
	void RecieveOverlayState(EOverlayState InOverlayState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ALS Interface")
	void RecieveCurrentGait(EGait InGait);

	UFUNCTION(BlueprintNativeEvent)
	void ToggleCrouch();
};
