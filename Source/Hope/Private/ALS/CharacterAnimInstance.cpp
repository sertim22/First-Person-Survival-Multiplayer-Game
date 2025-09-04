// Copyright Sertim all rights reserved


#include "ALS/CharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/CharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HopeInterfaces/CombatInterface.h"
#include "AnimationStateMachineLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "AbilitySystemGlobals.h"
#include "ALS/HopeCharacterMovementComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

void FCharacterAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	Owner = InAnimInstance->TryGetPawnOwner();
	if (!Owner) return;

	Character = Cast<ACharacterBase>(Owner);
	MovementComponent = Cast<UCharacterMovementComponent>(Owner->GetMovementComponent());
}

void FCharacterAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FCharacterAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

#if WITH_EDITOR
EDataValidationResult UCharacterAnimInstance::IsDataValid(FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	GameplayTagPropertyMap.IsDataValid(this, Context);

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif // WITH_EDITOR

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Proxy.Character) return;

	UHopeCharacterMovementComponent* CharMoveComp = CastChecked<UHopeCharacterMovementComponent>(Proxy.MovementComponent);
	const FHopeCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}

void UCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Owner) return;

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData(DeltaSeconds);
	UpdateVelocityData();
	UpdateAccelerationData();
	UpdateLocomotionData();
	UpdateRootYawOffset(DeltaSeconds);
	UpdateCharacterStates(DeltaSeconds);
	UpdateBlendWeightData(DeltaSeconds);
	UpdateWallDetectionHeuristic();
	bIsFirstUpdate = false;
}

UCharacterAnimInstance::UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCharacterAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	GameplayTagPropertyMap.Initialize(this, ASC);
}


/*
*		***********************	Update Functions ***********************
*/

void UCharacterAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(Proxy.Owner->GetActorLocation() - WorldLocation);
	WorldLocation = Proxy.Owner->GetActorLocation();
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
	if (bIsFirstUpdate)
	{
		DisplacementSinceLastUpdate = 0.f;
		DisplacementSpeed = 0.f;
	}
}

void UCharacterAnimInstance::UpdateRotationData(float DeltaSeconds)
{
	LastFrameActorYawRotation = ActorYawRotation;

	// Saving Current Yaw Rotation
	WorldRotation = Proxy.Owner->GetActorRotation();
	ActorYawRotation = WorldRotation.Yaw;
	DeltaActorYawRotation = ActorYawRotation - LastFrameActorYawRotation;

	// Calculate LeanAngle
	float SelectedMultiplier = 0.f;
	switch (VelocityLocomotionDirection)
	{
	case ELocomotionDirections::ELD_Backward:
		SelectedMultiplier = -1.f;
		break;
	case ELocomotionDirections::ELD_Forward:
		SelectedMultiplier = 1.f;
		break;
	case ELocomotionDirections::ELD_Left:
		SelectedMultiplier = 0.f;
		break;
	case ELocomotionDirections::ELD_Right:
		SelectedMultiplier = 0.f;
		break;
	}
	float AngleDegrees = (DeltaActorYawRotation / DeltaSeconds) / 5 * SelectedMultiplier;
	AdditiveLeanAngle = UKismetMathLibrary::ClampAngle(AngleDegrees, -90.f, 90.f);

	// Calculate AimPitch
	AimPitch = UKismetMathLibrary::NormalizeAxis(Proxy.Owner->GetBaseAimRotation().Pitch);

	if (bIsFirstUpdate)
	{
		DeltaActorYawRotation = 0.f;
		AdditiveLeanAngle = 0.f;
	}
}

void UCharacterAnimInstance::UpdateVelocityData()
{
	WorldVelocity = Proxy.Owner->GetVelocity();
	WorldVelocity2D = WorldVelocity * FVector(1.f, 1.f, 0.f);
	LocalVelocity2D = UKismetMathLibrary::LessLess_VectorRotator(WorldVelocity2D, WorldRotation);
	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);
	LocalVelocityDirectionAngleWithOffset = LocalVelocityDirectionAngle - RootYawOffset;
}

void UCharacterAnimInstance::UpdateAccelerationData()
{
	Acceleration = Proxy.MovementComponent->GetCurrentAcceleration();
	Acceleration2D = Acceleration * FVector(1.f, 1.f, 0.f);
	LocalAcceleration2D = UKismetMathLibrary::LessLess_VectorRotator(Acceleration2D, WorldRotation);
	bIsAccelerating = !UKismetMathLibrary::NearlyEqual_FloatFloat(UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D), 0.0f);
	PivotDirection2D = UKismetMathLibrary::Normal(UKismetMathLibrary::VLerp(PivotDirection2D,
		UKismetMathLibrary::Normal(Acceleration2D), 0.5f));
}

void UCharacterAnimInstance::UpdateLocomotionData()
{
	bWasMovingLastUpdate = !UKismetMathLibrary::Vector_IsZero(LocalVelocity2D);
	LastFrameVelocityLocomotionDirection = VelocityLocomotionDirection;

	VelocityLocomotionAngle = CalculateDirection(WorldVelocity2D, WorldRotation);
	VelocityLocomotionAngleWithOffset = VelocityLocomotionAngle - RootYawOffset;
	AccelerationLocomotionAngle = CalculateDirection(Acceleration2D, WorldRotation);

	VelocityLocomotionDirection = CalculateLocomotionDirectionFromAngle(VelocityLocomotionAngleWithOffset, LocomotionDirectionDeadZone,
		VelocityLocomotionDirection, bWasMovingLastUpdate);

	AccelerationLocomotionDirection = CalculateLocomotionDirectionFromAngle(AccelerationLocomotionAngle, LocomotionDirectionDeadZone,
		AccelerationLocomotionDirection, bWasMovingLastUpdate);
}

void UCharacterAnimInstance::UpdateRootYawOffset(float DeltaSeconds)
{
	if (RootYawOffsetMode == ERootYawOffsetMode::ERYOM_Accumulate)
	{
		SetRootYawOffset(RootYawOffset + (DeltaActorYawRotation * -1.f));
	}
	if (RootYawOffsetMode == ERootYawOffsetMode::ERYOM_BlendOut)
	{
		SetRootYawOffset(UKismetMathLibrary::FloatSpringInterp(RootYawOffset, 0.f, RootYawOffsetSpringState,
			80.f, 1.f, DeltaSeconds, 1.f, 0.5f));
	}

	RootYawOffsetMode = ERootYawOffsetMode::ERYOM_BlendOut;
}

void UCharacterAnimInstance::UpdateCharacterStates(float DeltaSeconds)
{
	// IsOnGround
	bIsOnGround = Proxy.MovementComponent->IsMovingOnGround();

	// Gait
	LastFrameGait = CurrentGait;
	CurrentGait = IncomingGait;
	bIsGaitChanged = CurrentGait != LastFrameGait;

	// Crouch
	bLastFrameIsCrouching = bIsCrouching;
	bIsCrouching = IncomingGait == EGait::EG_Crouching;
	bCrouchStateChanged = bIsCrouching != bLastFrameIsCrouching;

	// Fire
	if (GameplayTag_IsFiring)
	{
		TimeSinceFiredWeapon = 0.0f;
	}
	else
	{
		TimeSinceFiredWeapon = TimeSinceFiredWeapon + DeltaSeconds;
	}

	// Air
	bIsInAir = Proxy.MovementComponent->MovementMode == EMovementMode::MOVE_Falling;
	bIsJumping = WorldVelocity.Z > 0.0f;
	bIsFalling = WorldVelocity.Z < 0.0f;

	if (bIsJumping)
	{
		TimeToJumpApex = (0.0f - WorldVelocity.Z) / (Proxy.MovementComponent->GetGravityZ() * Proxy.MovementComponent->GravityScale);
	}
	else
	{
		TimeToJumpApex = 0.0f;
	}

	if (bIsFalling)
	{
		TimeFalling = TimeFalling + DeltaSeconds;
	}
	else if (bIsJumping)
	{
		TimeFalling = 0.f;
	}
}

void UCharacterAnimInstance::UpdateWallDetectionHeuristic()
{
	bIsRunningIntoWall = UKismetMathLibrary::VSizeXY(LocalAcceleration2D) > 0.1f
		&& UKismetMathLibrary::VSizeXY(LocalVelocity2D) < 200.f
		&& UKismetMathLibrary::InRange_FloatFloat(UKismetMathLibrary::Dot_VectorVector(
			UKismetMathLibrary::Normal(LocalAcceleration2D), UKismetMathLibrary::Normal(LocalVelocity2D)), -0.6f, 0.6f, true, true);
}

void UCharacterAnimInstance::UpdateBlendWeightData(float DeltaSeconds)
{
	UpperBodyDynamicAdditiveWeight = UKismetMathLibrary::SelectFloat(1.0f,
		UKismetMathLibrary::FInterpTo(UpperBodyDynamicAdditiveWeight, 0.0f, DeltaSeconds, 6.0f), IsAnyMontagePlaying() && bIsOnGround);
}

void UCharacterAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FAnimationStateResultReference AnimationState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, Result);
	if (UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
	{
		TurnYawCurveValue = 0.f;
	}
	else
	{
		RootYawOffsetMode = ERootYawOffsetMode::ERYOM_Accumulate;
		ProcessTurnYawCurve();
	}
}

void UCharacterAnimInstance::SetupStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	VelocityLocomotionDirection = LastFrameVelocityLocomotionDirection;
}

void UCharacterAnimInstance::UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FAnimationStateResultReference AnimationState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, Result);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
	{
		RootYawOffsetMode = ERootYawOffsetMode::ERYOM_Hold;
	}
}

void UCharacterAnimInstance::SetupPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotInitialDirection = VelocityLocomotionDirection;
}

void UCharacterAnimInstance::UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (LastPivotTime > 0.f)
	{
		LastPivotTime = LastPivotTime - UAnimExecutionContextLibrary::GetDeltaTime(Context);
	}
}

void UCharacterAnimInstance::UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FAnimationStateResultReference AnimationState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, Result);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
	{
		RootYawOffsetMode = ERootYawOffsetMode::ERYOM_Accumulate;
	}
}

/*
*		***********************	Update Functions ***********************
*/


UCharacterMovementComponent* UCharacterAnimInstance::GetCharacterMovement()
{
	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner)
	{
		return nullptr;
	}

	UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());
	if (CMC)
	{
		return CMC;
	}
	else return nullptr;
}

ELocomotionDirections UCharacterAnimInstance::CalculateLocomotionDirectionFromAngle(float Angle, float DeadZone, ELocomotionDirections CurrentDirection, bool UseCurrentDirection)
{
	float AbsAngle = UKismetMathLibrary::Abs(Angle);
	// Favor playing Fwd / Bwd anims over strafing anims.
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = FwdDeadZone;

	if (UseCurrentDirection) // If moving Fwd, double the Fwd dead zone.It should be harder to leave Fwd when moving Fwd.
	// When moving Left / Right, the dead zone will be smaller so we don't rapidly toggle between directions.
	{
		switch (CurrentDirection)
		{
		case ELocomotionDirections::ELD_Forward:
			FwdDeadZone = FwdDeadZone * 2; 
			break;
		case ELocomotionDirections::ELD_Backward:
			BwdDeadZone = BwdDeadZone * 2;
			break;
		default: 
			break;
		}
	}
	if (AbsAngle <= 45.0f + FwdDeadZone) // Forward
	{
		return ELocomotionDirections::ELD_Forward;
	}
	else if (AbsAngle >= 135.0f - BwdDeadZone) // Backward
	{
		return ELocomotionDirections::ELD_Backward;
	}
	else if (Angle > 0.0f) // Right
	{
		return ELocomotionDirections::ELD_Right;
	}
	else // Left
	{
		return ELocomotionDirections::ELD_Left;
	}
}

ELocomotionDirections UCharacterAnimInstance::GetOppositeDirection(ELocomotionDirections CurrentDirection)
{
	switch (CurrentDirection)
	{
	case ELocomotionDirections::ELD_Forward:
		return ELocomotionDirections::ELD_Backward;
	case ELocomotionDirections::ELD_Backward:
		return ELocomotionDirections::ELD_Forward;
	case ELocomotionDirections::ELD_Left:
		return ELocomotionDirections::ELD_Right;
	case ELocomotionDirections::ELD_Right:
		return ELocomotionDirections::ELD_Left;
	default:
		return ELocomotionDirections::ELD_Forward;
	}
}

bool UCharacterAnimInstance::IsMovingPerpendicularToInitialPivot()
{
	return (PivotInitialDirection == ELocomotionDirections::ELD_Forward || PivotInitialDirection == ELocomotionDirections::ELD_Backward) &&
		!(VelocityLocomotionDirection == ELocomotionDirections::ELD_Forward || VelocityLocomotionDirection == ELocomotionDirections::ELD_Backward)
		|| (PivotInitialDirection == ELocomotionDirections::ELD_Left || PivotInitialDirection == ELocomotionDirections::ELD_Right) &&
		!(VelocityLocomotionDirection == ELocomotionDirections::ELD_Left || VelocityLocomotionDirection == ELocomotionDirections::ELD_Right);
}

void UCharacterAnimInstance::SetRootYawOffset(float Angle)
{
	if (bEnableRootYawOffset)
	{
		FVector2D SelectedAngle;
		if (CurrentGait == EGait::EG_Crouching)
		{
			SelectedAngle = RootYawOffsetAngleClampCrouched;
		}
		else
		{
			SelectedAngle = RootYawOffsetAngleClamp;
		}
		bool PickA = SelectedAngle.X == SelectedAngle.Y;
		float InYawRootOffset = UKismetMathLibrary::NormalizeAxis(Angle);

		RootYawOffset = UKismetMathLibrary::SelectFloat(InYawRootOffset,
			UKismetMathLibrary::ClampAngle(InYawRootOffset, SelectedAngle.X, SelectedAngle.Y), PickA);
		AimYaw = RootYawOffset * (-1.0f);
	}
	else
	{
		RootYawOffset = 0.0f;
		AimYaw = 0.f;
	}
}

void UCharacterAnimInstance::ProcessTurnYawCurve()
{
	LastFrameTurnYawCurveValue = TurnYawCurveValue;

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(GetCurveValue(TurnYawWeightCurveName), 0.f, 1.e-4))
	{
		TurnYawCurveValue = 0.f;
		LastFrameTurnYawCurveValue = 0.f;
	}
	else
	{
		TurnYawCurveValue = UKismetMathLibrary::SafeDivide(GetCurveValue(RemainingTurnYawCurveName), GetCurveValue(TurnYawWeightCurveName));
		if (LastFrameTurnYawCurveValue != 0.f)
		{
			SetRootYawOffset(RootYawOffset - (TurnYawCurveValue - LastFrameTurnYawCurveValue));
		}
	}
}


/* ALS Interface */

void UCharacterAnimInstance::RecieveOverlayState_Implementation(EOverlayState InOverlayState)
{
	OverlayState = InOverlayState;
}

void UCharacterAnimInstance::RecieveCurrentGait_Implementation(EGait InGait)
{
	IncomingGait = InGait;
}

/* ALS Interface */
