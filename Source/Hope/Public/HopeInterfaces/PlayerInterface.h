// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

class APlayerCharacter;
class UOverlayWidget;
class USceneCaptureComponent2D;
class UBuildingComponent;
class UAbilitySystemComponent;
class UAttributeSet;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class HOPE_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/*Getters*/
	// Call this function to get PlayerCharacter Reference
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Interface|Getters", meta = (ReturnDisplayName = "Player"))
	APlayerCharacter* GetPlayerCharacterReference();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Interface|Getters")
	void RotateCharacterCapture(float X);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Interface|Getters")
	void ChangeCharacterCaptureZoom(float Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Interface|Getters")
	UBuildingComponent* GetPlayerBuildingComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Player Interface|Line Trace")
	FHitResult LineTraceFromCamera(float StartLocationMultiplier, float EndLocationMultiplier);

	UFUNCTION(BlueprintNativeEvent)
	UAbilitySystemComponent* GetPlayerASC();

	UFUNCTION(BlueprintNativeEvent)
	UAttributeSet* GetPlayerAS();
};
