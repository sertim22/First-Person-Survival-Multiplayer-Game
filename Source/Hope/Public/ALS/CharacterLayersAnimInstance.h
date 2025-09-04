// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ALS/ALSDataTypes.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "CharacterLayersAnimInstance.generated.h"

class UCharacterAnimInstance;

/**
 * Layers anim instance class connected to the "CharacterAnimInstance".
 */
UCLASS()
class HOPE_API UCharacterLayersAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "00.Setup", meta = (ReturnDisplayName = "ReturnValue"), DisplayName = "GetABPCharacterBase", meta = (BlueprintThreadSafe))
	UCharacterAnimInstance* GetCharacterAnimInstance();

	UFUNCTION(meta = (ThreadSafe))
	void UpdateBlendWeightData(float DeltaSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "00.Setup", EditDefaultsOnly)
	FName LocomotionDistanceCurveName = FName("Distance");

	/*
	*			************ Idle ***********
	*/

	UPROPERTY(EditAnywhere, Category = "01.Idle")
	UAnimSequence* IdleAnim;

	UPROPERTY(EditAnywhere, Category = "01.Idle")
	UAnimSequence* CrouchedIdleAnim;

	UPROPERTY(EditAnywhere, Category = "01.Idle")
	TArray<UAnimSequence*> IdleBreaksAnims;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "01.Idle")
	int32 CurrentIdleBreakIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "01.Idle")
	float IdleBreakDelayTime = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "01.Idle")
	float TimeUntilNextIdleBreak = 0.f;

	UFUNCTION(meta = (BlueprintThreadSafe))
	void ChooseIdleBreakDelayTime();

	UFUNCTION(meta = (BlueprintThreadSafe))
	void ResetIdleBreakTransitionLogic();

	UFUNCTION(meta = (BlueprintThreadSafe))
	void ProcessIdleBreakTransitionLogic(float DeltaTime);

	UFUNCTION(Category = "01.Idle", BlueprintCallable, meta = (BlueprintThreadSafe), BlueprintPure, meta = (ReturnDisplayName = "ReturnValue"))
	bool CanPlayIdleBreak();

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*			************ Idle ***********
	*/


	/*
	*			************ Start ***********
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Start")
	float StrideWarpingStartAlpha = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Start", EditDefaultsOnly)
	float StrideWarpingBlendInStartOffset = 0.15f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Start", EditDefaultsOnly)
	float StrideWarpingBlendInDurationScaled = 0.2f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Start", EditDefaultsOnly)
	FVector2D PlayRateClampStartsPivots = FVector2D(0.6f, 5.f);

	UPROPERTY(EditAnywhere, Category = "02.Start")
	FDirectionalAnimations WalkStartAnimations;

	UPROPERTY(EditAnywhere, Category = "02.Start")
	FDirectionalAnimations JogStartAnimations;

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*			************ Start ***********
	*/


	/*
	*			************ Cycle ***********
	*/

	UPROPERTY(EditAnywhere, Category = "03.Cycle")
	FDirectionalAnimations WalkCycleAnimations;

	UPROPERTY(EditAnywhere, Category = "03.Cycle")
	FDirectionalAnimations JogCycleAnimations;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Cycle", EditDefaultsOnly)
	FVector2D PlayRateClampCycle = FVector2D(0.8f, 1.2f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Cycle", EditDefaultsOnly)
	float StrideWarpingCycleAlpha = 0.15f;

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*			************ Cycle ***********
	*/


	/*
	*			************ Stop ***********
	*/

	UPROPERTY(EditAnywhere, Category = "04.Stop")
	FDirectionalAnimations WalkStopAnimations;

	UPROPERTY(EditAnywhere, Category = "04.Stop")
	FDirectionalAnimations JogStopAnimations;

	UFUNCTION(Category = "04.Stop", BlueprintCallable, meta = (BlueprintThreadSafe), BlueprintPure, meta = (ReturnDisplayName = "ReturnValue"))
	bool ShouldDistanceMatchStop();

	UFUNCTION(Category = "04.Stop", BlueprintCallable, meta = (BlueprintThreadSafe), BlueprintPure, meta = (ReturnDisplayName = "ReturnValue"))
	float GetPredictedStopDistance();

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*			************ Stop ***********
	*/


	/*
	*			************ Pivot ***********
	*/

	UPROPERTY(EditAnywhere, Category = "05.Pivot")
	FDirectionalAnimations WalkPivotAnimations;

	UPROPERTY(EditAnywhere, Category = "05.Pivot")
	FDirectionalAnimations JogPivotAnimations;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Pivot")
	FVector PivotStartingAcceleration = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Pivot")
	float StrideWarpingPivotAlpha = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Pivot")
	float TimeAtPivotStop = 0.f;

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "05.Pivot", BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe), meta = (ReturnDisplayName = "ReturnValue"))
	UAnimSequence* SelectPivotSequence(ELocomotionDirections InDirection);

	/*
	*			************ Pivot ***********
	*/


	/*
	*			************ TurnInPlace ***********
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Turn In Place")
	float TurnInPlaceRotationAngle = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Turn In Place")
	float TurnInPlaceRecoveryAngle = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Turn In Place")
	float TurnInPlaceTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "06.Turn In Place")
	UAnimSequence* TurnRight90Anim;

	UPROPERTY(EditAnywhere, Category = "06.Turn In Place")
	UAnimSequence* TurnLeft90Anim;

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceEntry(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceAnims(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "05.Pivot", BlueprintCallable, BlueprintPure, meta = (BlueprintThreadSafe), meta = (ReturnDisplayName = "ReturnValue"))
	UAnimSequence* SelectTurnInPlaceAnim(float InAngle);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*			************ TurnInPlace ***********
	*/


	/*
	*			************ Crouch ***********
	*/

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	UAnimSequence* CrouchEnterAnim;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	UAnimSequence* CrouchExitAnim;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	FDirectionalAnimations CrouchStartAnimations;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	FDirectionalAnimations CrouchCycleAnimations;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	FDirectionalAnimations CrouchStopAnimations;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	FDirectionalAnimations CrouchPivotAnimations;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	UAnimSequence* CrouchTurnRight90Anim;

	UPROPERTY(EditAnywhere, Category = "07.Crouch")
	UAnimSequence* CrouchTurnLeft90Anim;

	/*
	*			************ Crouch ***********
	*/


	/*
	*			************ Air ***********
	*/

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* JumpStartAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* JumpStartLoopAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* JumpApexAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* FallLoopAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* FallLandAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	UAnimSequence* FallLandAdditiveRecoveryAnim;

	UPROPERTY(Transient, BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	float LandRecoveryAlpha = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, EditAnywhere, Category = "08.Air")
	float TimeFalling = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Air", EditDefaultsOnly)
	FName JumpDistanceCurveName = FName("GroundDistance");

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "08.Air", meta = (BlueprintThreadSafe))
	void UpdateJumpFallData(float DeltaSeconds);

	/*
	*			************ Air ***********
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "09.Aim Offset")
	UBlendSpace* IdleAimOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "09.Aim Offset")
	UBlendSpace* RelaxedAimOffset;


	/*
	*			************ Skeletal Control ***********
	*/

	UPROPERTY(EditAnywhere, Category = "10.Skeletal Control", BlueprintReadOnly)
	UAnimSequence* LeftHandPoseOverride = nullptr;

	UFUNCTION(meta = (ThreadSafe), Category = "10.Skeletal Control")
	void UpdateSkeletalControlData();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control")
	float HandIK_RightAlpha = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control")
	float HandIK_LeftAlpha = 1.0f;

	UPROPERTY(Transient, EditAnywhere, Category = "10.Skeletal Control")
	bool DisableHandIK = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control", EditDefaultsOnly)
	FName DisableRHandIKCurveName = FName("DisableRHandIK");

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control", EditDefaultsOnly)
	FName DisableLHandIKCurveName = FName("DisableLHandIK");

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control")
	float LeftHandPoseOverrideWeight = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control", EditDefaultsOnly)
	bool bEnableLeftHandPoseOverride = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control", EditDefaultsOnly)
	FName DisableLeftHandPoseOverrideCurveName = FName("DisableLeftHandPoseOverride");

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "10.Skeletal Control", meta = (ReturnDisplayName = "ReturnValue"))
	bool ShouldEnableFootPlacement();

	/*
	*			************ Skeletal Control ***********
	*/


	/*
	*			************ Blend Weight Data ***********
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "11.Blend Weight Data")
	float HipFireUpperBodyOverrideWeight = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "11.Blend Weight Data")
	float AimOffsetBlendWeight = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "11.Blend Weight Data")
	bool bRaiseWeaponAfterFiringWhenCrouched = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "11.Blend Weight Data", EditDefaultsOnly)
	float RaiseWeaponAfterFiringDuration = 0.5f;

	UPROPERTY(Transient, Category = "11.Blend Weight Data", EditDefaultsOnly)
	FName ApplyHipfireOverridePoseCurveName = FName("applyHipfireOverridePose");

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "11.Blend Weight Data")
	UAnimSequence* AimHipFirePoseAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "11.Blend Weight Data")
	UAnimSequence* AimHipFirePoseCrouchAnim;

	/*
	*			************ Blend Weight Data ***********
	*/
};
