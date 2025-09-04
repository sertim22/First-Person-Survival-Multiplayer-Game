// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/Player/HopePlayerState.h"
#include "AbilitySystem/HopeAbilitySystemComponent.h"
#include "InputControl/HopePlayerController.h"
#include "UI/HUD/HopeHUD.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Image.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Building/BuildingComponent.h"

APlayerCharacter::APlayerCharacter()
{
	/*Camera Settings*/
	this->bUseControllerRotationYaw = true;
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->TargetArmLength = 0.0f;
	CameraSpringArm->SetupAttachment(GetMesh());
	CameraSpringArm->bUsePawnControlRotation = true;
	CameraSpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("neck_02"));
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CameraSpringArm);
	PlayerCamera->bUsePawnControlRotation = true;
	/*Camera Settings end*/

	BuildingComponent = CreateDefaultSubobject<UBuildingComponent>(TEXT("BuildingComponent"));
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeCharacterAbilitySystem();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitializeCharacterAbilitySystem();
}

FHitResult APlayerCharacter::LineTraceFromCamera_Implementation(float StartLocationMultiplier, float EndLocationMultiplier)
{
	FVector Start = PlayerCamera->GetComponentLocation() + PlayerCamera->GetForwardVector() * StartLocationMultiplier;
	FVector End = PlayerCamera->GetComponentLocation() + PlayerCamera->GetForwardVector() * EndLocationMultiplier;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	FHitResult HitResult;

	UKismetSystemLibrary::LineTraceSingle(PlayerCamera,
		Start,
		End,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Gray);
	return HitResult;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set BuildingComponent variables
	BuildingComponent->Camera = PlayerCamera;
}

void APlayerCharacter::ToggleAim_Client()
{
	Super::ToggleAim_Client();
}

void APlayerCharacter::InitializeCharacterAbilitySystem()
{
	AHopePlayerState* HopePlayerState = GetPlayerState<AHopePlayerState>();
	check(HopePlayerState);
	HopePlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(HopePlayerState, this);
	Cast<UHopeAbilitySystemComponent>(HopePlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = HopePlayerState->GetAbilitySystemComponent();
	AttributeSet = HopePlayerState->GetAttributeSet();
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}
