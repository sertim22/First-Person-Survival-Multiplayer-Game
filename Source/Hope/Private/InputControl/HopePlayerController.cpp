// Fill out your copyright notice in the Description page of Project Settings.


#include "InputControl/HopePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputControl/HopeInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/HopeAbilitySystemComponent.h"
#include "InputActionValue.h"
#include "HopeInterfaces/PlayerInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Building/BuildingComponent.h"
#include "HopeInterfaces/BuildInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "HopeInterfaces/ALSInterface.h"
#include "HopeInterfaces/CombatInterface.h"
#include "UI/HUD/HopeHUD.h"
// Inventory Plugin
#include "Inv_FunctionLibrary.h"
#include "Widgets/Inv_Widget.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"


AHopePlayerController::AHopePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	ItemTraceChannel = ECC_GameTraceChannel1;
}

void AHopePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UInv_FunctionLibrary::TraceForItem(this, ItemTraceLength, ItemTraceChannel, ThisActor, LastActor, Inv_Widget);
}

void AHopePlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(DefaultMappingContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	InventoryComponent = FindComponentByClass<UInv_InventoryComponent>();

	CreateHUDWidget();
}

void AHopePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* HopeInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	/*Locomotion*/
	HopeInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHopePlayerController::Move);
	HopeInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHopePlayerController::Look);
	HopeInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHopePlayerController::Jump);
	HopeInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHopePlayerController::StopJump);
	HopeInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AHopePlayerController::Aim);
	HopeInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AHopePlayerController::Aim);
	HopeInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHopePlayerController::Crouch);
	/*Locomotion end*/

	/*Building Actions*/
	HopeInputComponent->BindAction(ToggleBuildingModeAction, ETriggerEvent::Started, this, &AHopePlayerController::EnableBuildingMode);
	HopeInputComponent->BindAction(SwitchBuildingUpAction, ETriggerEvent::Started, this, &AHopePlayerController::SwitchBuildingUp);
	HopeInputComponent->BindAction(SwitchBuildingDownAction, ETriggerEvent::Started, this, &AHopePlayerController::SwitchBuildingDown);
	HopeInputComponent->BindAction(PlaceBuildingAction, ETriggerEvent::Started, this, &AHopePlayerController::PlaceBuilding);
	HopeInputComponent->BindAction(RotateBuildingAction, ETriggerEvent::Triggered, this, &AHopePlayerController::RotateBuilding);
	/*Building Actions end*/

	/*Inventory*/
	HopeInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &AHopePlayerController::Interact);
	HopeInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AHopePlayerController::ToggleInventory);
	/*Inventory*/
}

UHopeAbilitySystemComponent* AHopePlayerController::GetASC()
{
	if (HopeAbilitySystemComponent == nullptr)
	{
		HopeAbilitySystemComponent = Cast<UHopeAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return HopeAbilitySystemComponent;
}

void AHopePlayerController::Move(const FInputActionValue& Value)
{
	const FVector2d InputAxisVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AHopePlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D InputAxisVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddControllerYawInput(InputAxisVector.X);
		ControlledPawn->AddControllerPitchInput(InputAxisVector.Y);
	}
}

void AHopePlayerController::Jump()
{
	Cast<ACharacter>(GetPawn())->Jump();
}

void AHopePlayerController::StopJump()
{
	Cast<ACharacter>(GetPawn())->StopJumping();
}

void AHopePlayerController::Aim()
{
	ICombatInterface::Execute_ToggleAim(GetPawn());
}

void AHopePlayerController::Crouch()
{
	IALSInterface::Execute_ToggleCrouch(GetPawn());
}

void AHopePlayerController::EnableBuildingMode()
{
	IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn())->StartBuildMode();
}

void AHopePlayerController::SwitchBuildingUp()
{
	UBuildingComponent* BuildingComponent = IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn());
	BuildingComponent->BuildID = FMath::Clamp(BuildingComponent->BuildID + 1, 0, BuildingComponent->Buildables.Num() - 1);
	BuildingComponent->ChangeBuildGhostMesh();
}

void AHopePlayerController::SwitchBuildingDown()
{
	UBuildingComponent* BuildingComponent = IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn());
	BuildingComponent->BuildID = FMath::Clamp(BuildingComponent->BuildID - 1, 0, BuildingComponent->Buildables.Num() - 1);
	BuildingComponent->ChangeBuildGhostMesh();
}

void AHopePlayerController::PlaceBuilding()
{
	UBuildingComponent* BuildingComponent = IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn());
	if (BuildingComponent->bIsBuildModeOn && BuildingComponent->bCanBuild)
	{
		BuildingComponent->SpawnBuilding_Server(BuildingComponent->Buildables[BuildingComponent->BuildID]->BuildingClass,
			BuildingComponent->BuildTransform, GetPawn(), GetPawn());
	}
}

void AHopePlayerController::RotateBuilding(const FInputActionValue& Value)
{
	UBuildingComponent* BuildingComponent = IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn());
	float InputFloatValue = Value.Get<float>();
	if(InputFloatValue == 1) BuildingComponent->RotateBuildGhostMesh(BuildingRotationSpeed);
	else BuildingComponent->RotateBuildGhostMesh(-BuildingRotationSpeed);
}

void AHopePlayerController::Interact()
{
	if (!ThisActor.IsValid()) return;

	UInv_ItemComponent* ItemComp = ThisActor->FindComponentByClass<UInv_ItemComponent>();
	if (!IsValid(ItemComp) || !InventoryComponent.IsValid()) return;

	InventoryComponent->TryAddItem(ItemComp);


	/*FHitResult HitResult = IPlayerInterface::Execute_LineTraceFromCamera(GetPawn(), 1.f, 350.f);
	AActor* HitActor = HitResult.GetActor();
	if (HitActor)
	{
		if (HitActor->Implements<UBuildInterface>()) // Case 0: Interaction with Building
		{
			IPlayerInterface::Execute_GetPlayerBuildingComponent(GetPawn())->InteractWithBuilding_Server();
		}
	}*/
}

void AHopePlayerController::ToggleInventory()
{
	if (!InventoryComponent.IsValid()) return;
	InventoryComponent->ToggleInventoryMenu();

	//if (InventoryComponent->IsMenuOpen())
	//{
		// TODO: hide unused widgets if needed
	//}
	//else
	//{
		// TODO: show unused widgets if needed
	//}
}

void AHopePlayerController::AddApplicableMappingContext(EDesiredMappingContext DesiredMappingContext)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		switch (DesiredMappingContext)
		{
		case EDesiredMappingContext::EDM_Default:
			Subsystem->RemoveMappingContext(InventoryMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			break;
		case EDesiredMappingContext::EDM_Inventory:
			Subsystem->RemoveMappingContext(DefaultMappingContext);
			Subsystem->AddMappingContext(InventoryMappingContext, 0);
			break;
		}
	}
}

void AHopePlayerController::CreateHUDWidget()
{
	if (IsLocalController())
	{
		if (AHopeHUD* HopeHUD = Cast<AHopeHUD>(this->GetHUD()))
		{
			HopeHUD->InitializeWidgets(this);
			Inv_Widget = HopeHUD->Inv_Widget; // Inv_Widget.
		}
	}
}
