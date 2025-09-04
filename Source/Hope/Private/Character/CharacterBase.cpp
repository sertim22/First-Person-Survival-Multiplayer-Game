// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "HopeInterfaces/ALSInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "ALS/HopeCharacterMovementComponent.h"

static FName NAME_HopeCharacterCollisionProfile_Capsule(TEXT("HopePawnCapsule"));
static FName NAME_HopeCharacterCollisionProfile_Mesh(TEXT("HopePawnMesh"));

ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UHopeCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NetCullDistanceSquared = 900000000.0f;

	// Capsule
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_HopeCharacterCollisionProfile_Capsule);
	CapsuleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Mesh
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_HopeCharacterCollisionProfile_Mesh);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Movement
	UHopeCharacterMovementComponent* HopeMoveComp = CastChecked<UHopeCharacterMovementComponent>(GetCharacterMovement());
	HopeMoveComp->GravityScale = 1.0f;
	HopeMoveComp->MaxAcceleration = 2400.0f;
	HopeMoveComp->BrakingFrictionFactor = 1.0f;
	HopeMoveComp->BrakingFriction = 6.0f;
	HopeMoveComp->GroundFriction = 8.0f;
	HopeMoveComp->BrakingDecelerationWalking = 1400.0f;
	HopeMoveComp->bUseControllerDesiredRotation = false;
	HopeMoveComp->bOrientRotationToMovement = false;
	HopeMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	HopeMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	HopeMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	HopeMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	HopeMoveComp->SetCrouchedHalfHeight(65.0f);
	HopeMoveComp->JumpZVelocity = 400.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ALS
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
	DOREPLIFETIME(ACharacterBase, OverlayState);
	DOREPLIFETIME(ACharacterBase, CurrentGait);
}

void ACharacterBase::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ACharacterBase::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			// Only call FastSharedReplication if data has changed since the last frame.
			// Skipping this call will cause replication to reuse the same bunch that we previously
			// produced, but not send it to clients that already received. (But a new client who has not received
			// it, will get it this frame)
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				ReplicatedMovementMode = SharedMovement.RepMovementMode;

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}

	// We cannot fastrep right now. Don't send anything.
	return false;
}

void ACharacterBase::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		ReplicatedServerLastTransformUpdateTimeStamp = SharedRepMovement.RepTimeStamp;

		// Movement mode
		if (ReplicatedMovementMode != SharedRepMovement.RepMovementMode)
		{
			ReplicatedMovementMode = SharedRepMovement.RepMovementMode;
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Jump force
		bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;

		// Crouch
		if (bIsCrouched != SharedRepMovement.bIsCrouched)
		{
			bIsCrouched = SharedRepMovement.bIsCrouched;
			OnRep_IsCrouched();
		}
	}
}

void ACharacterBase::UpdateCurrentGait_Server_Implementation(EGait Gait)
{
	UpdateCurrentGait_Client(Gait);
}

void ACharacterBase::UpdateCurrentGait_Client_Implementation(EGait Gait)
{
	CurrentGait = Gait;
	IALSInterface::Execute_RecieveCurrentGait(GetMesh()->GetAnimInstance(), CurrentGait);

	switch (CurrentGait)
	{
	case EGait::EG_Walking:
		switch (OverlayState)
		{
		case EOverlayState::EOS_Default:
			SetGaitSettings(GaitSettingsDataTable->FindRow<FGaitSettings>("DefaultWalk", TEXT("")));
			break;
		}
		break;
	case EGait::EG_Jogging:
		switch (OverlayState)
		{
		case EOverlayState::EOS_Default:
			SetGaitSettings(GaitSettingsDataTable->FindRow<FGaitSettings>("DefaultJog", TEXT("")));
			break;
		}
		break;
	case EGait::EG_Crouching:
		switch (OverlayState)
		{
		case EOverlayState::EOS_Default:
			GetCharacterMovement()->MaxWalkSpeedCrouched = GaitSettingsDataTable->FindRow<FGaitSettings>("DefaultCrouch", TEXT(""))->MaxWalkSpeed;
			SetGaitSettings(GaitSettingsDataTable->FindRow<FGaitSettings>("DefaultCrouch", TEXT("")));
			break;
		}
		break;
	case EGait::EG_Sprinting:
		break;
	}
}

void ACharacterBase::SetGaitSettings(FGaitSettings* InSettings)
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	CMC->MaxWalkSpeed = InSettings->MaxWalkSpeed;
	CMC->MaxAcceleration = InSettings->MaxAcceleration;
	CMC->BrakingDecelerationWalking = InSettings->BrakingDeceleration;
	CMC->BrakingFrictionFactor = InSettings->BrakingFrictionFactor;
	CMC->BrakingFriction = InSettings->BrakingFriction;
	CMC->bUseSeparateBrakingFriction = InSettings->UseSeparateBrakingFriction;
}

void ACharacterBase::ToggleCrouch_Server_Implementation()
{
	ToggleCrouch_Client();
}

void ACharacterBase::ToggleCrouch_Client_Implementation()
{
	const UHopeCharacterMovementComponent* HopeMoveComp = CastChecked<UHopeCharacterMovementComponent>(GetCharacterMovement());

	if (bIsCrouched || HopeMoveComp->bWantsToCrouch)
	{
		UpdateCurrentGait_Server(EGait::EG_Jogging);
		UnCrouch();
	}
	else if (HopeMoveComp->IsMovingOnGround())
	{
		UpdateCurrentGait_Server(EGait::EG_Crouching);
		Crouch();
	}
}

void ACharacterBase::ToggleAim_Server_Implementation()
{
	ToggleAim_Client();
}

void ACharacterBase::ToggleAim_Client_Implementation()
{
	if (!bIsAiming)
	{
		switch (CurrentGait)
		{
		case EGait::EG_Crouching:
			UpdateCurrentGait_Server(EGait::EG_Crouching);
			break;
		default:
			UpdateCurrentGait_Server(EGait::EG_Walking);
			break;
		}
		bIsAiming = true;
	}
	else
	{
		switch (CurrentGait)
		{
		case EGait::EG_Crouching:
			UpdateCurrentGait_Server(EGait::EG_Crouching);
			break;
		default:
			UpdateCurrentGait_Server(EGait::EG_Jogging);
			break;
		}
		bIsAiming = false;
	}
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UpdateCurrentGait_Server(EGait::EG_Jogging);
}

void ACharacterBase::InitializeCharacterAbilitySystem()
{

}

void ACharacterBase::OnRep_ReplicatedAcceleration()
{
	if (UHopeCharacterMovementComponent* HopeMovementComponent = Cast<UHopeCharacterMovementComponent>(GetCharacterMovement()))
	{
		// Decompress Acceleration
		const double MaxAccel = HopeMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		HopeMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->bProxyIsJumpForceApplied || (Character->JumpForceTimeRemaining > 0.0f);
		bIsCrouched = Character->bIsCrouched;

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}

	if (bIsCrouched != Other.bIsCrouched)
	{
		return false;
	}

	return true;
}

bool FSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;
	Ar << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}
