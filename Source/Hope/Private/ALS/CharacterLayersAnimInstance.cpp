// Copyright Sertim all rights reserved


#include "ALS/CharacterLayersAnimInstance.h"
#include "ALS/CharacterAnimInstance.h"
#include "SequencePlayerLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "AnimExecutionContextLibrary.h"
#include "AnimationStateMachineLibrary.h"

void UCharacterLayersAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (GetCharacterAnimInstance())
	{
		UpdateJumpFallData(DeltaSeconds);
		UpdateSkeletalControlData();
	}
}

UCharacterAnimInstance* UCharacterLayersAnimInstance::GetCharacterAnimInstance()
{
	UCharacterAnimInstance* CAI = Cast<UCharacterAnimInstance>(GetOwningComponent()->GetAnimInstance());
	if (CAI) return CAI;
	else return nullptr;
}

void UCharacterLayersAnimInstance::UpdateBlendWeightData(float DeltaSeconds)
{
	if (!bRaiseWeaponAfterFiringWhenCrouched && GetCharacterAnimInstance()->bIsCrouching ||
		GetCharacterAnimInstance()->bIsCrouching && GetCharacterAnimInstance()->bIsOnGround)
	{
		HipFireUpperBodyOverrideWeight = 0.0f;
		AimOffsetBlendWeight = 1.0f;
	}
	else if (GetCharacterAnimInstance()->TimeSinceFiredWeapon < RaiseWeaponAfterFiringDuration ||
		(GetCharacterAnimInstance()->bIsCrouching || !GetCharacterAnimInstance()->bIsOnGround) ||
		GetCurveValue(ApplyHipfireOverridePoseCurveName) > 0.0f)
	{
		HipFireUpperBodyOverrideWeight = 1.0f;
		AimOffsetBlendWeight = 1.0f;
	}
	else
	{
		HipFireUpperBodyOverrideWeight = UKismetMathLibrary::FInterpTo(HipFireUpperBodyOverrideWeight, 0.0f, DeltaSeconds, 1.0f);
		// Use aiming aim offset when we are idle or we have root yaw offset. Use relaxed aim offset during regular motion.
		bool PickA = UKismetMathLibrary::Abs(GetCharacterAnimInstance()->RootYawOffset) < 10.0f && GetCharacterAnimInstance()->bIsAccelerating;
		AimOffsetBlendWeight = UKismetMathLibrary::FInterpTo(AimOffsetBlendWeight,
			UKismetMathLibrary::SelectFloat(HipFireUpperBodyOverrideWeight, 1.0f, PickA), DeltaSeconds, 10.0f);
	}
}

void UCharacterLayersAnimInstance::UpdateSkeletalControlData()
{
	HandIK_RightAlpha = UKismetMathLibrary::FClamp(UKismetMathLibrary::SelectFloat(
		0.0f, 1.0f, DisableHandIK) - GetCurveValue(DisableRHandIKCurveName), 0.0f, 1.0f);
	HandIK_LeftAlpha = UKismetMathLibrary::FClamp(UKismetMathLibrary::SelectFloat(
		0.0f, 1.0f, DisableHandIK) - GetCurveValue(DisableLHandIKCurveName), 0.0f, 1.0f);
}

void UCharacterLayersAnimInstance::SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	LeftHandPoseOverrideWeight = UKismetMathLibrary::FClamp((UKismetMathLibrary::SelectFloat(1.0f, 0.0f,
		bEnableLeftHandPoseOverride) - GetCurveValue(DisableLeftHandPoseOverrideCurveName)), 0.0f, 1.0f);
}

bool UCharacterLayersAnimInstance::ShouldEnableFootPlacement()
{
	if (UCharacterAnimInstance* CAI = GetCharacterAnimInstance())
	{
		return CAI->GetCurveValue(CAI->DisableLegIKCurveName) <= 0.0f && GetCharacterAnimInstance()->bUseFootPlacement;
	}
	else
	{
		return false;
	}
}

void UCharacterLayersAnimInstance::UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	if (GetCharacterAnimInstance()->bIsCrouching)
	{
		SelectedAnimSequence = AimHipFirePoseCrouchAnim;
	}
	else
	{
		SelectedAnimSequence = AimHipFirePoseAnim;
	}
	USequenceEvaluatorLibrary::SetSequence(USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result), SelectedAnimSequence);
}

void UCharacterLayersAnimInstance::ChooseIdleBreakDelayTime()
{
	IdleBreakDelayTime = 6 + UKismetMathLibrary::Percent_IntInt(UKismetMathLibrary::FTrunc(
		UKismetMathLibrary::Abs(GetCharacterAnimInstance()->WorldLocation.X + GetCharacterAnimInstance()->WorldLocation.Y)), 10);
}

void UCharacterLayersAnimInstance::ResetIdleBreakTransitionLogic()
{
	TimeUntilNextIdleBreak = IdleBreakDelayTime;
}

void UCharacterLayersAnimInstance::ProcessIdleBreakTransitionLogic(float DeltaTime)
{
	if (CanPlayIdleBreak())
	{
		TimeUntilNextIdleBreak = TimeUntilNextIdleBreak - DeltaTime;
	}
	else
	{
		ResetIdleBreakTransitionLogic();
	}
}

bool UCharacterLayersAnimInstance::CanPlayIdleBreak()
{
	return IdleBreaksAnims.Num() > 0 && !(GetCharacterAnimInstance()->bIsCrouching || GetCharacterAnimInstance()->bIsJumping);
}

void UCharacterLayersAnimInstance::SetupIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	ChooseIdleBreakDelayTime();
	ResetIdleBreakTransitionLogic();
}

void UCharacterLayersAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FAnimationStateResultReference AnimationState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, Result);
	if (!UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState))
	{
		ProcessIdleBreakTransitionLogic(UAnimExecutionContextLibrary::GetDeltaTime(Context));
	}
}

void UCharacterLayersAnimInstance::UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	if (GetCharacterAnimInstance()->bIsCrouching) SelectedAnimSequence = CrouchedIdleAnim;
	else SelectedAnimSequence = IdleAnim;

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result), SelectedAnimSequence);
}

void UCharacterLayersAnimInstance::SetupIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	if (GetCharacterAnimInstance()->bIsCrouching) SelectedAnimSequence = CrouchEnterAnim;
	else SelectedAnimSequence = CrouchExitAnim;

	USequencePlayerLibrary::SetSequence(USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result), SelectedAnimSequence);
}

void UCharacterLayersAnimInstance::SetupIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	USequencePlayerLibrary::SetSequence(SequencePlayer, IdleBreaksAnims[CurrentIdleBreakIndex]);
	CurrentIdleBreakIndex++;
	if (CurrentIdleBreakIndex >= IdleBreaksAnims.Num())
	{
		CurrentIdleBreakIndex = 0;
	}
}

void UCharacterLayersAnimInstance::SetupStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	switch (GetCharacterAnimInstance()->CurrentGait)
	{
	case EGait::EG_Walking:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = WalkStartAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = WalkStartAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = WalkStartAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = WalkStartAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Jogging:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = JogStartAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = JogStartAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = JogStartAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = JogStartAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Crouching:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = CrouchStartAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = CrouchStartAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = CrouchStartAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = CrouchStartAnimations.Right;
			break;
		}
		break;
	}

	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectedAnimSequence);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);
	StrideWarpingStartAlpha = 0.f;
}

void UCharacterLayersAnimInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);
	StrideWarpingStartAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - StrideWarpingBlendInStartOffset, 0.f,
		StrideWarpingBlendInDurationScaled, 0.f, 1.f);

	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context,
		SequenceEvaluator,
		GetCharacterAnimInstance()->DisplacementSinceLastUpdate,
		LocomotionDistanceCurveName,
		FVector2D(UKismetMathLibrary::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, StrideWarpingStartAlpha),
		PlayRateClampStartsPivots.Y));
}

void UCharacterLayersAnimInstance::UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimSequence* SelectedAnimSequence = nullptr;
	FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	switch (GetCharacterAnimInstance()->CurrentGait)
	{
	case EGait::EG_Walking:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = WalkCycleAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = WalkCycleAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = WalkCycleAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = WalkCycleAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Jogging:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = JogCycleAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = JogCycleAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = JogCycleAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = JogCycleAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Crouching:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = CrouchCycleAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = CrouchCycleAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = CrouchCycleAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = CrouchCycleAnimations.Right;
			break;
		}
		break;
	}

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, SelectedAnimSequence);
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, GetCharacterAnimInstance()->DisplacementSpeed, PlayRateClampCycle);
	
	StrideWarpingCycleAlpha = UKismetMathLibrary::FInterpTo(StrideWarpingCycleAlpha,
		UKismetMathLibrary::SelectFloat(0.5f, 1.f, GetCharacterAnimInstance()->bIsRunningIntoWall),
		UAnimExecutionContextLibrary::GetDeltaTime(Context), 10.f);
}

bool UCharacterLayersAnimInstance::ShouldDistanceMatchStop()
{
	return !UKismetMathLibrary::NearlyEqual_FloatFloat(UKismetMathLibrary::VSizeXYSquared(GetCharacterAnimInstance()->LocalVelocity2D), 0.f)
		&& !GetCharacterAnimInstance()->bIsAccelerating;
}

float UCharacterLayersAnimInstance::GetPredictedStopDistance()
{
	return UKismetMathLibrary::VSizeXY(UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		GetCharacterAnimInstance()->GetCharacterMovement()->GetLastUpdateVelocity(),
		GetCharacterAnimInstance()->GetCharacterMovement()->bUseSeparateBrakingFriction,
		GetCharacterAnimInstance()->GetCharacterMovement()->BrakingFriction,
		GetCharacterAnimInstance()->GetCharacterMovement()->GroundFriction,
		GetCharacterAnimInstance()->GetCharacterMovement()->BrakingFrictionFactor,
		GetCharacterAnimInstance()->GetCharacterMovement()->BrakingDecelerationWalking));
}

void UCharacterLayersAnimInstance::SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	UAnimSequence* SelectedAnimSequence = nullptr;

	switch (GetCharacterAnimInstance()->CurrentGait)
	{
	case EGait::EG_Walking:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = WalkStopAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = WalkStopAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = WalkStopAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = WalkStopAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Jogging:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = JogStopAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = JogStopAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = JogStopAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = JogStopAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Crouching:
		switch ((GetCharacterAnimInstance()->VelocityLocomotionDirection))
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = CrouchStopAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = CrouchStopAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = CrouchStopAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = CrouchStopAnimations.Right;
			break;
		}
		break;
	}

	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectedAnimSequence);
	if (ShouldDistanceMatchStop() == false) // If we got here, and we can't distance match a stop on start, match to 0 distance
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, 0.f, LocomotionDistanceCurveName);
	}
}

void UCharacterLayersAnimInstance::UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	if (ShouldDistanceMatchStop())
	{
		float DistanceToMatch = GetPredictedStopDistance();
		if (DistanceToMatch > 0.f) // Distance Match to the stop point
		{
			UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToMatch, LocomotionDistanceCurveName);
		}
		else
		{
			// Advanced time naturally once the character has come to a stop or the animation reaches zero speed.
			USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator);
		}
	}
	else
	{
		// Advanced time naturally once the character has come to a stop or the animation reaches zero speed.
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator);
	}
}

void UCharacterLayersAnimInstance::SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	PivotStartingAcceleration = GetCharacterAnimInstance()->LocalAcceleration2D;

	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, SelectPivotSequence((GetCharacterAnimInstance()->AccelerationLocomotionDirection)));
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);
	StrideWarpingPivotAlpha = 0.f;
	TimeAtPivotStop = 0.f;
	GetCharacterAnimInstance()->LastPivotTime = 0.2f;
}

void UCharacterLayersAnimInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);

	if (GetCharacterAnimInstance()->LastPivotTime > 0.0f) // Allow switching the selected pivot for a short duration at the beginning.
	{
		UAnimSequence* NewDesiredSequence = SelectPivotSequence(GetCharacterAnimInstance()->AccelerationLocomotionDirection);
		if (NewDesiredSequence != USequenceEvaluatorLibrary::GetSequence(SequenceEvaluator))
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluator, NewDesiredSequence);
			PivotStartingAcceleration = GetCharacterAnimInstance()->LocalAcceleration2D;
		}
	}

	if (UKismetMathLibrary::Dot_VectorVector(GetCharacterAnimInstance()->LocalVelocity2D,
		GetCharacterAnimInstance()->LocalAcceleration2D) < 0.f) // Does acceleration oppose velocity?
	{
		// While acceleration opposes velocity, the character is still approaching the pivot point,
		// so we distance match to that point.
		float DistanceToMatch = UKismetMathLibrary::VSizeXY(UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(
			GetCharacterAnimInstance()->GetCharacterMovement()->GetCurrentAcceleration(),
			GetCharacterAnimInstance()->GetCharacterMovement()->GetLastUpdateVelocity(),
			GetCharacterAnimInstance()->GetCharacterMovement()->GroundFriction));
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, DistanceToMatch, LocomotionDistanceCurveName);
		TimeAtPivotStop = ExplicitTime;
	}
	else 
	{
		// Alpha = (ExplicitTime - StopTime - Offset)/Duration.
		// We want the blend in to start after we've already stopped, and just started accelerating
		StrideWarpingPivotAlpha = UKismetMathLibrary::MapRangeClamped(ExplicitTime - TimeAtPivotStop - StrideWarpingBlendInStartOffset,
			0.f, StrideWarpingBlendInDurationScaled, 0.f, 1.f);

		// Once acceleration and velocity are aligned, the character is accelerating away from the pivot point,
		// so we just advance time by distance traveled for the rest of the animation.
		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(Context,
			SequenceEvaluator,
			GetCharacterAnimInstance()->DisplacementSinceLastUpdate,
			LocomotionDistanceCurveName,
			FVector2D(UKismetMathLibrary::Lerp(0.2f, PlayRateClampStartsPivots.X, StrideWarpingPivotAlpha), PlayRateClampStartsPivots.Y)); // Smoothly increase the minimum playrate speed, as we blend in stride warping
	}
}

UAnimSequence* UCharacterLayersAnimInstance::SelectPivotSequence(ELocomotionDirections InDirection)
{
	UAnimSequence* SelectedAnimSequence = nullptr;

	switch (GetCharacterAnimInstance()->CurrentGait)
	{
	case EGait::EG_Walking:
		switch (InDirection)
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = WalkPivotAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = WalkPivotAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = WalkPivotAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = WalkPivotAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Jogging:
		switch (InDirection)
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = JogPivotAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = JogPivotAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = JogPivotAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = JogPivotAnimations.Right;
			break;
		}
		break;
	case EGait::EG_Crouching:
		switch (InDirection)
		{
		case ELocomotionDirections::ELD_Backward:
			SelectedAnimSequence = CrouchPivotAnimations.Backward;
			break;
		case ELocomotionDirections::ELD_Forward:
			SelectedAnimSequence = CrouchPivotAnimations.Forward;
			break;
		case ELocomotionDirections::ELD_Left:
			SelectedAnimSequence = CrouchPivotAnimations.Left;
			break;
		case ELocomotionDirections::ELD_Right:
			SelectedAnimSequence = CrouchPivotAnimations.Right;
			break;
		}
		break;
	}

	return SelectedAnimSequence;
}

void UCharacterLayersAnimInstance::SetupTurnInPlaceEntry(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRotationAngle = UKismetMathLibrary::SignOfFloat(GetCharacterAnimInstance()->RootYawOffset) * (-1);
}

void UCharacterLayersAnimInstance::SetupTurnInPlaceAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	TurnInPlaceTime = 0.f;
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);
}

void UCharacterLayersAnimInstance::UpdateTurnInPlaceAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluator,
		SelectTurnInPlaceAnim(TurnInPlaceRotationAngle));
	TurnInPlaceTime = TurnInPlaceTime + UAnimExecutionContextLibrary::GetDeltaTime(Context);

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, TurnInPlaceTime);
}

UAnimSequence* UCharacterLayersAnimInstance::SelectTurnInPlaceAnim(float InAngle)
{
	if (InAngle > 0.f)
	{
		if (GetCharacterAnimInstance()->bIsCrouching)
		{
			return CrouchTurnRight90Anim;
		}
		else
		{
			return TurnRight90Anim;
		}
	}
	else
	{
		if (GetCharacterAnimInstance()->bIsCrouching)
		{
			return CrouchTurnLeft90Anim;
		}
		else
		{
			return TurnLeft90Anim;
		}
	}

	/*bool bIsWideRange = UKismetMathLibrary::InRange_FloatFloat(UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizeAxis(
		GetCharacterAnimInstance()->RootYawOffset)), 90.f, 180.f);

	if (GetCharacterAnimInstance()->CurrentGait == EGait::EG_Crouching)
	{
		if (bIsWideRange)
		{
			if (ShouldTurnLeft) return CrouchTurnLeft180Anim;
			else return CrouchTurnRight180Anim;
		}
		else
		{
			if (ShouldTurnLeft) return CrouchTurnLeft90Anim;
			else return CrouchTurnRight90Anim;
		}
	}
	else
	{
		if (bIsWideRange)
		{
			if (ShouldTurnLeft) return TurnLeft180Anim;
			else return TurnRight180Anim;
		}
		else
		{
			if (ShouldTurnLeft) return TurnLeft90Anim;
			else return TurnRight90Anim;
		}
	}*/
}

void UCharacterLayersAnimInstance::SetupTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceRecoveryAngle = TurnInPlaceRotationAngle;
}

void UCharacterLayersAnimInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result),
		SelectTurnInPlaceAnim(TurnInPlaceRecoveryAngle));
}

void UCharacterLayersAnimInstance::LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const float ClampedTime = UKismetMathLibrary::MapRangeClamped(TimeFalling, 0.0f, 0.4f, 0.1f, 1.0f);
	LandRecoveryAlpha = UKismetMathLibrary::SelectFloat(ClampedTime * 0.5f, ClampedTime, GetCharacterAnimInstance()->bIsCrouching);
}

void UCharacterLayersAnimInstance::SetupFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	USequenceEvaluatorLibrary::SetExplicitTime(USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result), 0.0f);
}

void UCharacterLayersAnimInstance::UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result),
		GetCharacterAnimInstance()->GroundDistance, JumpDistanceCurveName);
}

void UCharacterLayersAnimInstance::UpdateJumpFallData(float DeltaSeconds)
{
	if (GetCharacterAnimInstance()->bIsFalling)
	{
		TimeFalling = TimeFalling + DeltaSeconds;
	}
	else if (GetCharacterAnimInstance()->bIsJumping)
	{
		TimeFalling = 0.0f;
	}
}
