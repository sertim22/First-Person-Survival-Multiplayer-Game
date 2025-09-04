// Copyright Sertim all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildingComponent.generated.h"

class UCameraComponent;
class ABuildableBase;

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	EBT_Foundation UMETA(DisplayName = "Foundation"),
	EBT_Pillar UMETA(DisplayName = "Pillar"),
	EBT_Wall UMETA(DisplayName = "Wall"),
	EBT_Doorway UMETA(DisplayName = "Doorway"),
	EBT_WindowWall UMETA(DisplayName = "WindowWall"),
	EBT_Ceiling UMETA(DisplayName = "Ceiling"),
	EBT_Ramp UMETA(DisplayName = "Ramp"),
	EBT_Door UMETA(DisplayName = "Door"),
	EBT_Window UMETA(DisplayName = "Window")
};

USTRUCT(BlueprintType)
struct HOPE_API FBuildables : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buildables")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buildables")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = ETraceTypeQuery::TraceTypeQuery1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buildables")
	TSubclassOf<ABuildableBase> BuildingClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buildables")
	EBuildingType BuildingType = EBuildingType::EBT_Foundation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class HOPE_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UBuildingComponent();

	bool bIsBuildModeOn = false;
	bool bCanBuild = false;

	UPROPERTY(Replicated)
	FTransform BuildTransform;

	UCameraComponent* Camera = nullptr;

	TArray<const FBuildables*> Buildables;

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 BuildID = 0;

	void ChangeBuildGhostMesh();

	UFUNCTION(Server, Reliable)
	void SpawnBuilding_Server(TSubclassOf<ABuildableBase> BuildingClass, const FTransform& Transform, AActor* Owner, APawn* Instigator);

	void RotateBuildGhostMesh(float YawRotation);

	UFUNCTION(BlueprintCallable, Category = "Building System")
	void StartBuildMode();

	UFUNCTION(Server, Reliable)
	void InteractWithBuilding_Server();

protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void SpawnBuildGhostComponent();

	void StopBuildMode();

	UPROPERTY(BlueprintReadOnly, Replicated)
	UStaticMeshComponent* BuildGhostComponent = nullptr;

	// Sets "bCanBuild" based on value passed in.
	void GiveBuildColor(bool bIsGreen);
	// This function sets the Transform of the "BuildGhostComponent" and checks the conditions for building.
	void SetBuildGhostComponentTransformAndColor();
	// This function decides whether to allow building or not for the "InBuildingType".
	void DefineConditionsForBuilding(FHitResult& HitResult, EBuildingType InBuildingType);
	/* Called to update "BuildGhostComponent" transform and check conditions for building.
	Its frequency can be tweaked in the Editor via changing "UpdateBuildGhostComponentTransformAndColorTime" variable
	located at the player's BuildingComponent.*/
	void UpdateBuildGhostComponentTransformAndColor();

	UFUNCTION(NetMulticast, Reliable)
	void InteractWithBuilding_Client(AActor* InBuilding);

	FTimerHandle BuildTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Building System|Colors")
	UMaterialInterface* BuildingIsAllowedColor;

	UPROPERTY(EditAnywhere, Category = "Building System|Colors")
	UMaterialInterface* BuildingIsNotAllowedColor;

	UPROPERTY(EditAnywhere, Category = "Building System|Update Building")
	float LineTraceForBuilding = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Building System|Update Building")
	float UpdateBuildGhostComponentTransformAndColorTime = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category = "Building System")
	TObjectPtr<UDataTable> BuildablesDataTable;

	bool DetectBuildBoxes();

	AActor* HitActor;
	UPrimitiveComponent* HitComponent;

	bool IsBuildingSupported(bool bSupportedByBuilding);
	bool IsBuildingColliding();

	UPROPERTY(EditAnywhere, Category = "Building System|Update Building")
	float BuildingSupportHeight = 70.f;

	UPROPERTY(EditAnywhere, Category = "Building System|Tags")
	FName PillarTagName = FName("Pillar");
};
