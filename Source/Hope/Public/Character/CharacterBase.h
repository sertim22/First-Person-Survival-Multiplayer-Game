// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "HopeInterfaces/CombatInterface.h"
#include "HopeInterfaces/ALSInterface.h"
#include "ALS/ALSDataTypes.h"
#include "CharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UStaticMesh;
class USkeletalMesh;

/**
 * FHopeReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FHopeReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0; // Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0; // Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class HOPE_API ACharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface, public IALSInterface
{
	GENERATED_BODY()

public:

	ACharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FOnASCRegistered OnASCRegistered;


	/*
	*		***********************	02.ALS ***********************
	*/

	/* Used for defining what animation layer to play. */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "02.ALS|Overlay State")
	EOverlayState OverlayState = EOverlayState::EOS_Default;

	/* Used for defining Gait Settings such as MaxWalkSpeed, Acceleration, Braking and etc. */
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "02.ALS|Gait")
	EGait CurrentGait = EGait::EG_Jogging;

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	virtual bool UpdateSharedReplication();

	/* ALS Interface functions */

	virtual void ToggleCrouch_Implementation() override { ToggleCrouch_Server(); }

	/* ALS Interface functions end */

	/*
	*		***********************	02.ALS ***********************
	*/



	/*
	*		***********************	05.Combat ***********************
	*/

	UPROPERTY(BlueprintReadWrite, Category = "05.Combat")
	bool bIsAiming = false;

	/*Combat Interface*/

	virtual void ToggleAim_Implementation() override { ToggleAim_Server(); }

	/*Combat Interface*/

	/*
	*		***********************	05.Combat ***********************
	*/

protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;


	/*
	*		***********************	02.ALS ***********************
	*/

	/* Assign this variable so that Movement Settings can change depending on Gait and Overlay State. */
	UPROPERTY(EditDefaultsOnly, Category = "02.ALS|Gait")
	TObjectPtr<UDataTable> GaitSettingsDataTable;

	// Updates CharacterMovement variables according to the GaitSettings Data Table.
	UFUNCTION(Server, Reliable)
	void UpdateCurrentGait_Server(EGait Gait);

	UFUNCTION(NetMulticast, Reliable)
	void UpdateCurrentGait_Client(EGait Gait);

	// Modifies Character Movement variables according to the Settings passed in.
	void SetGaitSettings(FGaitSettings* InSettings);

	// Enter Crouching Gait State by calling this function.
	UFUNCTION(Server, Reliable)
	void ToggleCrouch_Server();

	UFUNCTION(NetMulticast, Reliable)
	void ToggleCrouch_Client();

	/*
	*		***********************	02.ALS ***********************
	*/



	/*
	*		***********************	05.Combat ***********************
	*/

	// Enable Aiming by calling this function.
	UFUNCTION(Server, Reliable)
	void ToggleAim_Server();

	UFUNCTION(NetMulticast, Reliable)
	virtual void ToggleAim_Client();

	/*
	*		***********************	05.Combat ***********************
	*/
	

private:

	// Override this function in child classes
	virtual void InitializeCharacterAbilitySystem();


	/*
	*		***********************	02.ALS ***********************
	*/

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FHopeReplicatedAcceleration ReplicatedAcceleration;

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

	/*
	*		***********************	02.ALS ***********************
	*/
};
