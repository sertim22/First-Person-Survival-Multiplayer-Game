// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ALS/ALSDataTypes.h"
#include "HopeInterfaces/ALSInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/AnimInstanceProxy.h"
#include "GameplayEffectTypes.h"
#include "CharacterAnimInstance.generated.h"

class UCharacterMovementComponent;
class ACharacterBase;
class UAbilitySystemComponent;

USTRUCT()
struct FCharacterAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

protected:

	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;

public:

	// Pawn Owner of the "UCharacterAnimInstance" class.
	UPROPERTY(Transient)
	APawn* Owner = nullptr;

	// Reference to the "ACharacterBase" class to access its variables.
	UPROPERTY(Transient)
	class ACharacterBase* Character = nullptr;

	// Reference to the "UCharacterMovementComponent" of the "Character".
	UPROPERTY(Transient)
	class UCharacterMovementComponent* MovementComponent = nullptr;

};


/**
 * Anim instance class used for animating humanoid characters
 */
UCLASS(Config = Game)
class HOPE_API UCharacterAnimInstance : public UAnimInstance, public IALSInterface
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(Transient)
	FCharacterAnimInstanceProxy Proxy;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &Proxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	// Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
	// These should be used instead of manually querying for the gameplay tags.
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

public:

	UCharacterAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	/*
	*		***********************	Update Functions ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "00.Setup")
	bool bIsFirstUpdate = true;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "00.Setup")
	float UpperBodyDynamicAdditiveWeight = 0.0f;

	UFUNCTION(meta = (ThreadSafe))
	void UpdateLocationData(float DeltaSeconds);

	UFUNCTION(meta = (ThreadSafe))
	void UpdateRotationData(float DeltaSeconds);

	UFUNCTION(meta = (ThreadSafe))
	void UpdateVelocityData();

	UFUNCTION(meta = (ThreadSafe))
	void UpdateAccelerationData();
	
	UFUNCTION(meta = (ThreadSafe))
	void UpdateLocomotionData();

	UFUNCTION(meta = (ThreadSafe))
	void UpdateRootYawOffset(float DeltaSeconds);

	UFUNCTION(meta = (ThreadSafe))
	void UpdateCharacterStates(float DeltaSeconds);

	UFUNCTION(meta = (ThreadSafe))
	void UpdateWallDetectionHeuristic();

	UFUNCTION(meta = (ThreadSafe))
	void UpdateBlendWeightData(float DeltaSeconds);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetupPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(Category = "StateNodeFunctions", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/*
	*		***********************	Update Functions ***********************
	*/

	
	/*
	*		***********************	02.Location Data ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Location Data")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Location Data")
	float DisplacementSinceLastUpdate = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "02.Location Data")
	float DisplacementSpeed = 0.f;

	/*
	*		***********************	02.Location Data ***********************
	*/


	/*
	*		***********************	03.Rotation Data ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float ActorYawRotation = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float LastFrameActorYawRotation = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float DeltaActorYawRotation = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float AdditiveLeanAngle = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float AimPitch = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "03.Rotation Data")
	float AimYaw = 0.f;

	/*
	*		***********************	03.Rotation Data ***********************
	*/


	/*
	*		***********************	04.Velocity Data ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "04.Velocity Data")
	FVector WorldVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "04.Velocity Data")
	FVector WorldVelocity2D = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "04.Velocity Data")
	FVector LocalVelocity2D = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "04.Velocity Data")
	float LocalVelocityDirectionAngle = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "04.Velocity Data")
	float LocalVelocityDirectionAngleWithOffset = 0.f;

	/*
	*		***********************	04.Velocity Data ***********************
	*/


	/*
	*		***********************	05.Acceleration Data ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	FVector Acceleration2D = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	FVector LocalAcceleration2D = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	FVector PivotDirection2D = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	ELocomotionDirections PivotInitialDirection = ELocomotionDirections::ELD_Forward;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	float LastPivotTime = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	bool bIsAccelerating = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "05.Acceleration Data")
	bool bIsRunningIntoWall = false;

	/*
	*		***********************	05.Acceleration Data ***********************
	*/


	/*
	*		***********************	06.Locomotion Data ***********************
	*/

	// This pure function returns UCharacterMovementComponent.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "06.Locomotion Data", meta = (ReturnDisplayName = "ReturnValue"), meta=(BlueprintTrheadSafe))
	UCharacterMovementComponent* GetCharacterMovement();

	UFUNCTION(meta = (ThreadSafe))
	ELocomotionDirections CalculateLocomotionDirectionFromAngle(float Angle, float DeadZone, ELocomotionDirections CurrentDirection, bool UseCurrentDirection);

	UFUNCTION(meta = (ThreadSafe))
	ELocomotionDirections GetOppositeDirection(ELocomotionDirections CurrentDirection);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "06.Locomotion Data", meta = (ReturnDisplayName = "ReturnValue"), meta=(BlueprintThreadSafe))
	bool IsMovingPerpendicularToInitialPivot();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	ELocomotionDirections VelocityLocomotionDirection = ELocomotionDirections::ELD_Forward;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	ELocomotionDirections LastFrameVelocityLocomotionDirection = ELocomotionDirections::ELD_Forward;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	ELocomotionDirections AccelerationLocomotionDirection = ELocomotionDirections::ELD_Forward;

	// Angle that shows in what 2D direction character is moving.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	float VelocityLocomotionAngle = 0.f;

	// Angle that shows in what 2D direction character is accelerating.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	float AccelerationLocomotionAngle = 0.f;

	// Ange (VelocityLocomotionAngle - Root Yaw Offset) that shows in what 2D direction character is moving.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	float VelocityLocomotionAngleWithOffset = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data")
	bool bWasMovingLastUpdate = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "06.Locomotion Data", EditDefaultsOnly)
	float LocomotionDirectionDeadZone = 10.0f;

	/*
	*		***********************	06.Locomotion Data ***********************
	*/


	/*
	*		***********************	07.Root Yaw Offset ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	bool bEnableRootYawOffset = true;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.0f, 100.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	FVector2D RootYawOffsetAngleClampCrouched = FVector2D(-90.0f, 80.0f);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = "07.Root Yaw Offset")
	void SetRootYawOffset(float Angle);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = "07.Root Yaw Offset")
	void ProcessTurnYawCurve();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	float RootYawOffset = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	ERootYawOffsetMode RootYawOffsetMode = ERootYawOffsetMode::ERYOM_BlendOut;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	FFloatSpringState RootYawOffsetSpringState;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	float TurnYawCurveValue = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	float LastFrameTurnYawCurveValue = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	FName TurnYawWeightCurveName = FName("TurnYawWeight");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "07.Root Yaw Offset")
	FName RemainingTurnYawCurveName = FName("RemainingTurnYaw");

	/*
	*		***********************	07.Root Yaw Offset ***********************
	*/


	/*
	*		***********************	08.Character States ***********************
	*/

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Overlay State")
	EOverlayState OverlayState;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States")
	bool bUseFootPlacement = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States")
	float TimeSinceFiredWeapon = 9999.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States")
	bool GameplayTag_IsFiring = false;

	/* Gait */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Gait")
	EGait CurrentGait;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Gait")
	EGait IncomingGait;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Gait")
	EGait LastFrameGait;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Gait")
	bool bIsGaitChanged = false;
	/* Gait */

	/* Crouch */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Crouch")
	bool bIsCrouching = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Crouch")
	bool bLastFrameIsCrouching = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Crouch")
	bool bCrouchStateChanged = false;
	/* Crouch */

	/* Air */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	bool bIsJumping = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	bool bIsFalling = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	bool bIsInAir = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	float TimeToJumpApex = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	float GroundDistance = -1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	float TimeFalling = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "08.Character States|Air")
	bool bIsOnGround = false;
	/* Air */

	UPROPERTY(Transient, BlueprintReadOnly, Category = "10.Skeletal Control", EditDefaultsOnly)
	FName DisableLegIKCurveName = FName("DisableLegIK");

	/*
	*		***********************	08.Character States ***********************
	*/


	/* ALS Interface */

	virtual void RecieveOverlayState_Implementation(EOverlayState InOverlayState) override;
	virtual void RecieveCurrentGait_Implementation(EGait InGait) override;

	/* ALS Interface end*/
};
