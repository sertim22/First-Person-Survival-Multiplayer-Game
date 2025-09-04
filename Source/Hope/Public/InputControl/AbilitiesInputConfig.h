// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilitiesInputConfig.generated.h"

// Struct containing InputAction and InputTag variables that must be set in blueprints
USTRUCT(BlueprintType)
struct FCustodianInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

// Use this class to create InputConfig Data Asset to add Input Actions for the player abilites.
// If you do not add Input Action to this data asset the ability you created won't activate!
UCLASS()
class HOPE_API UAbilitiesInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// Returns correct InputAction for the InputTag passed in
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	// Array that contains all input actions for the Player Character abilities.
	// Don't forget to add Input Action here after creating a new ability with new Input Tag!
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FCustodianInputAction> AbilityInputActions;
};
