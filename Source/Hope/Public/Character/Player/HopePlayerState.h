// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "HopePlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

// This is a Player State class that implements IAbilitySystemInterface and must
// have a Blueprint version in the editor and assigned in GameMode blueprint class.
UCLASS()
class HOPE_API AHopePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AHopePlayerState();

	// Returns AbilitySystemComponent variable of this class
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// Returns AttributeSet variable of this class
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// Getters
	FORCEINLINE int32 GetPlayerLevel() const { return  Level; }

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:

	UPROPERTY(VisibleAnywhere, Category = "Player Level")
	int32 Level = 1;
};
