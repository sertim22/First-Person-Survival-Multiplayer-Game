// Copyright Sertim all rights reserved

#pragma once

#include "ALSDataTypes.generated.h"

class UAnimSequence;

/**
 * Singleton containing enums and structs connected to the Advanced Locomotion System
 */

UENUM(BlueprintType)
enum class EGait : uint8
{
	EG_Walking UMETA(DisplayName = "Walking"),
	EG_Jogging UMETA(DisplayName = "Jogging"),
	EG_Crouching UMETA(DisplayName = "Crouching"),
	EG_Sprinting UMETA(DisplayName = "Sprinting")
};

UENUM(BlueprintType)
enum class EOverlayState : uint8
{
	EOS_Default UMETA(DisplayName = "Default")
};

UENUM(BlueprintType)
enum class ELocomotionDirections : uint8
{
	ELD_Backward UMETA(DisplayName = "Backward"),
	ELD_Forward UMETA(DisplayName = "Forward"),
	ELD_Left UMETA(DisplayName = "Left"),
	ELD_Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class ERootYawOffsetMode : uint8
{
	ERYOM_BlendOut UMETA(DisplayName = "BlendOut"),
	ERYOM_Hold UMETA(DisplayName = "Hold"),
	ERYOM_Accumulate UMETA(DisplayName = "Accumulate")
};

USTRUCT(BlueprintType)
struct HOPE_API FDirectionalAnimations
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* Backward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* Forward = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* Right = nullptr;
};

USTRUCT(BlueprintType)
struct HOPE_API FGaitSettings : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxAcceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BrakingDeceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BrakingFrictionFactor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BrakingFriction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool UseSeparateBrakingFriction = false;
};