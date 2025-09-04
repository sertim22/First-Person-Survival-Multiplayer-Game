// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "HopeInterfaces/PlayerInterface.h"

#include "HopePlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UAbilitiesInputConfig;
class UHopeAbilitySystemComponent;
class UInv_Widget;
class UInv_InventoryComponent;
class AInv_ProxyMesh;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnProxyMeshReady, AInv_ProxyMesh*);

// Enum created to help choosing mapping context in "AddApplicableMappingContext" function
enum class EDesiredMappingContext : uint8
{
	EDM_Default,
	EDM_Inventory
};

// Player Controller class used for controlling PlayerCharacter Input.
// Add new InputActions and Functions here if you want to create a new
// InputControl.
UCLASS()
class HOPE_API AHopePlayerController : public APlayerController, public IPlayerInterface
{
	GENERATED_BODY()
	
public:

	AHopePlayerController();
	
	virtual void Tick(float DeltaSeconds) override;

protected:
	
	// Grant DefaultMappingContext at the start
	virtual void BeginPlay() override;
	// Bind InputActions to the Functions
	virtual void SetupInputComponent() override;

private:

	// Assign this variable in "BP_HopePlayerController" in order to be able
	// to activate Gameplay Abilities with the correct pressed input button.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAbilitiesInputConfig> AbilitiesInputConfig;

	// Reference to the Player's Ability System Component
	UPROPERTY()
	TObjectPtr<UHopeAbilitySystemComponent> HopeAbilitySystemComponent;
	// Returns casted HopeAbilitySystemComponent
	UHopeAbilitySystemComponent* GetASC();



	/*Mapping Contexts*/

	UPROPERTY(EditDefaultsOnly, Category = "Input|Mapping Contexts")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Mapping Contexts")
	TObjectPtr<UInputMappingContext> InventoryMappingContext;

	/*Mapping Contexts end*/



	/*Input Actions*/

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Locomotion")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Locomotion")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Locomotion")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Locomotion")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Locomotion")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Building")
	TObjectPtr<UInputAction> ToggleBuildingModeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Building")
	TObjectPtr<UInputAction> SwitchBuildingUpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Building")
	TObjectPtr<UInputAction> SwitchBuildingDownAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Building")
	TObjectPtr<UInputAction> PlaceBuildingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Building")
	TObjectPtr<UInputAction> RotateBuildingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Input Actions|Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	/*Input Actions end*/

	/*Input Functions*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void StopJump();
	void Aim();
	void Crouch();
	void EnableBuildingMode();
	void SwitchBuildingUp();
	void SwitchBuildingDown();
	void PlaceBuilding();
	void RotateBuilding(const FInputActionValue& Value);
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();
	/*Input Functions end*/



	/*
	* Building
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Building")
	float BuildingRotationSpeed = 1.f;

	/*
	* Building
	*/



	/*
	* Inventory
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float ItemTraceLength = 500.0f;

	TWeakObjectPtr<AActor> ThisActor;
	TWeakObjectPtr<AActor> LastActor;

	// Reference to the main Inventory Widget that contains all other Inventory Widgets.
	UPROPERTY()
	TObjectPtr<UInv_Widget> Inv_Widget;

	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;

	/*
	* Inventory
	*/



	// Checks current character's stance to add a mapping context you need.
	void AddApplicableMappingContext(EDesiredMappingContext DesiredMappingContext);

	// Call this function in BeginPlay to setup User Interface.
	void CreateHUDWidget();
};
