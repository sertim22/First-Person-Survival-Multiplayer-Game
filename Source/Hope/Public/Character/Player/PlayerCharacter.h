// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "HopeInterfaces/PlayerInterface.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UInventoryComponent;
class UWidgetComponent;
class USpringArmComponent;
class UBuildingComponent;

// Player Character Class which is derived from ACharacterBase. 
// Modify this class if you want to change settings for the 
// PlayerCharacter Components or override inherited functions.
UCLASS()
class HOPE_API APlayerCharacter : public ACharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
	
public:

	// Player Character Constructor
	APlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USpringArmComponent> CameraSpringArm;

	// Camera Component for the Player Character
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UCameraComponent> PlayerCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "04.Building System")
	TObjectPtr<UBuildingComponent> BuildingComponent;

	/*Player Interface*/
	virtual APlayerCharacter* GetPlayerCharacterReference_Implementation() override { return this; }
	virtual UBuildingComponent* GetPlayerBuildingComponent_Implementation() override { return BuildingComponent; }
	virtual FHitResult LineTraceFromCamera_Implementation(float StartLocationMultiplier, float EndLocationMultiplier) override;
	virtual UAbilitySystemComponent* GetPlayerASC_Implementation() override { return AbilitySystemComponent; }
	virtual UAttributeSet* GetPlayerAS_Implementation() override { return AttributeSet; }
	/*Player Interface end*/

protected:

	virtual void BeginPlay() override;

	virtual void ToggleAim_Client() override;

private:

	// This function must be called in "PossessedBy" and "OnRep_PlayerState",
	// to Initialize Ability System and Overlay for the Player Character
	virtual void InitializeCharacterAbilitySystem() override;
};
