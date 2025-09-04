// Copyright Sertim all rights reserved


#include "Building/BuildingComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HopeInterfaces/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HopeInterfaces/BuildInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Building/BuildableBase.h"

UBuildingComponent::UBuildingComponent()
{
	
}

void UBuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<FName> OutRowNames;
	UDataTableFunctionLibrary::GetDataTableRowNames(BuildablesDataTable, OutRowNames);
	for (FName RowName : OutRowNames)
	{
		const FBuildables* FoundRow = BuildablesDataTable->FindRow<FBuildables>(RowName, TEXT(""));
		Buildables.Add(FoundRow);
	}
}

void UBuildingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBuildingComponent, BuildTransform);
	DOREPLIFETIME(UBuildingComponent, BuildGhostComponent);
	DOREPLIFETIME(UBuildingComponent, BuildID);
}

void UBuildingComponent::SpawnBuildGhostComponent()
{
	BuildGhostComponent = NewObject<UStaticMeshComponent>(GetOwner(), UStaticMeshComponent::StaticClass());
	BuildGhostComponent->RegisterComponent();

	if (BuildGhostComponent) BuildGhostComponent->SetRelativeTransform(BuildTransform);

	BuildGhostComponent->SetStaticMesh(Buildables[BuildID]->Mesh);
	BuildGhostComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BuildGhostComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
}

void UBuildingComponent::GiveBuildColor(bool bIsBuildingAllowed)
{
	bCanBuild = bIsBuildingAllowed;
	for (int32 i = 0; i < BuildGhostComponent->GetNumMaterials(); i++)
	{
		if (bIsBuildingAllowed) BuildGhostComponent->SetMaterial(i, BuildingIsAllowedColor);
		else BuildGhostComponent->SetMaterial(i, BuildingIsNotAllowedColor);
	}
	BuildGhostComponent->SetWorldTransform(BuildTransform);
}

void UBuildingComponent::SetBuildGhostComponentTransformAndColor()
{
	FVector Start = Camera->GetComponentLocation() + Camera->GetForwardVector() * 10.f;
	FVector End = Camera->GetComponentLocation() + Camera->GetForwardVector() * LineTraceForBuilding;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::LineTraceSingle(Camera,
		Start,
		End,
		Buildables[BuildID]->TraceChannel,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true,
		FLinearColor::Black);

	if (bHit)
	{
		DefineConditionsForBuilding(HitResult, Buildables[BuildID]->BuildingType);
	}
	else
	{
		BuildTransform = FTransform(BuildTransform.GetRotation(), HitResult.TraceEnd, BuildTransform.GetScale3D());
		if (BuildGhostComponent) GiveBuildColor(false);
		else SpawnBuildGhostComponent();
	}
}

void UBuildingComponent::DefineConditionsForBuilding(FHitResult& HitResult, EBuildingType InBuildingType)
{
	BuildTransform = FTransform(BuildTransform.GetRotation(), HitResult.ImpactPoint, BuildTransform.GetScale3D());
	HitActor = HitResult.GetActor();
	HitComponent = HitResult.GetComponent();

	if (BuildGhostComponent)
	{
		bool bIsSnapBoxDetected = DetectBuildBoxes();
		bool bIsGhostMeshColliding = IsBuildingColliding();
		bool bShoudBeSupportedWithBuilding = false;
		switch (InBuildingType)
		{
		case EBuildingType::EBT_Foundation:
			BuildingSupportHeight = 150.f;
			bShoudBeSupportedWithBuilding = false;
			break;
		case EBuildingType::EBT_Pillar:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_Wall:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_Doorway:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_WindowWall:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_Ceiling:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_Ramp:
			BuildingSupportHeight = 70.f;
			bShoudBeSupportedWithBuilding = false;
			break;
		case EBuildingType::EBT_Door:
			bShoudBeSupportedWithBuilding = true;
			break;
		case EBuildingType::EBT_Window:
			bShoudBeSupportedWithBuilding = true;
			break;
		}
		bool bIsGhostMeshSupported = IsBuildingSupported(bShoudBeSupportedWithBuilding);
		if (bIsSnapBoxDetected) // Snap Box detected. Attach "GhostMeshComponent" to its Transform.
		{
			if (!bIsGhostMeshColliding) // "GhostMeshComponent" is not colliding with other Objects.
			{
				if (bIsGhostMeshSupported) // "GhostMeshComponent" is not floating in the air and does not break building logic.
				{
					GiveBuildColor(true);
				}
				else GiveBuildColor(false);
			}
			else GiveBuildColor(false);
		}
		else // No Snap Box was detected.
		{
			if (!bIsGhostMeshColliding && bIsGhostMeshSupported)
			{
				GiveBuildColor(true);
			}
			else GiveBuildColor(false);
		}
	}
	else
	{
		SpawnBuildGhostComponent();
	}
}

void UBuildingComponent::UpdateBuildGhostComponentTransformAndColor()
{
	if (!BuildTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(BuildTimerHandle, this, &UBuildingComponent::SetBuildGhostComponentTransformAndColor,
			UpdateBuildGhostComponentTransformAndColorTime, true);
	}
}

void UBuildingComponent::StopBuildMode()
{
	GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);
	bIsBuildModeOn = false;
	bCanBuild = false;
	if (BuildGhostComponent)
	{
		BuildGhostComponent->UnregisterComponent();
		BuildGhostComponent->DestroyComponent();
		BuildGhostComponent = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("Build Mode was Deactivated!"));
}

void UBuildingComponent::StartBuildMode()
{
	if (bIsBuildModeOn)
	{
		StopBuildMode();
	}
	else
	{
		bIsBuildModeOn = true;
		SetBuildGhostComponentTransformAndColor(); // Check Conditions before placing BuildGhostComponent
		UpdateBuildGhostComponentTransformAndColor(); // Set Timer to move BuildGhostComponent in Space and change its Color

		UE_LOG(LogTemp, Warning, TEXT("Build Mode was Activated!"));
	}
}

void UBuildingComponent::InteractWithBuilding_Server_Implementation()
{
	FHitResult ServerHitResult = IPlayerInterface::Execute_LineTraceFromCamera(GetOwner(), 1.f, 350.f);
	AActor* HitBuilding = ServerHitResult.GetActor();

	if (HitBuilding && HitBuilding->Implements<UBuildInterface>())
	{
		InteractWithBuilding_Client(HitBuilding);
	}
}

void UBuildingComponent::InteractWithBuilding_Client_Implementation(AActor* InBuilding)
{
	if (!InBuilding || !InBuilding->Implements<UBuildInterface>()) return;
	IBuildInterface::Execute_InteractWithBuilding(InBuilding);
}

bool UBuildingComponent::DetectBuildBoxes()
{
	bool bFound = false;
	
	if (HitActor && HitActor->Implements<UBuildInterface>())
	{
		TArray<UBoxComponent*> BoxesArray = IBuildInterface::Execute_ReturnBoxes(HitActor);
		for (UBoxComponent* CurrentBox : BoxesArray)
		{
			if (HitComponent == CurrentBox)
			{
				bFound = true;
				BuildTransform = FTransform(HitComponent->GetComponentRotation(), HitComponent->GetComponentLocation(),
					BuildTransform.GetScale3D());
				break;
			}
		}
	}
	return bFound;
}

bool UBuildingComponent::IsBuildingSupported(bool bSupportedByBuilding)
{
	if (bSupportedByBuilding)
	{
		TArray<AActor*> OverlappingActors;
		TSubclassOf<ABuildableBase> BBClass;
		BuildGhostComponent->GetOverlappingActors(OverlappingActors, BBClass);
		TArray<AActor*> RequiredActors;
		int32 NumPillars;
		switch (Buildables[BuildID]->BuildingType)
		{
		case EBuildingType::EBT_Wall:
			for (AActor* OverlappingActor : OverlappingActors)
			{
				if (OverlappingActor->ActorHasTag(PillarTagName))
				{
					RequiredActors.Add(OverlappingActor);
				}
			}
			NumPillars = RequiredActors.Num();
			//UE_LOG(LogTemp, Warning, TEXT("NumPillars = %d"), NumPillars);
			if (NumPillars == 2) return true;
			else return false;
			break;
		case EBuildingType::EBT_WindowWall:
			for (AActor* OverlappingActor : OverlappingActors)
			{
				if (OverlappingActor->ActorHasTag(PillarTagName))
				{
					RequiredActors.Add(OverlappingActor);
				}
			}
			NumPillars = RequiredActors.Num();
			//UE_LOG(LogTemp, Warning, TEXT("NumPillars = %d"), NumPillars);
			if (NumPillars == 2) return true;
			else return false;
			break;
		case EBuildingType::EBT_Doorway:
			for (AActor* OverlappingActor : OverlappingActors)
			{
				if (OverlappingActor->ActorHasTag(PillarTagName))
				{
					RequiredActors.Add(OverlappingActor);
				}
			}
			NumPillars = RequiredActors.Num();
			//UE_LOG(LogTemp, Warning, TEXT("NumPillars = %d"), NumPillars);
			if (NumPillars == 2) return true;
			else return false;
			break;
		case EBuildingType::EBT_Ceiling:
			return true;
			break;
		case EBuildingType::EBT_Ramp:
			return true;
			break;
		case EBuildingType::EBT_Door:
			return true;
			break;
		case EBuildingType::EBT_Window:
			return true;
			break;
		case EBuildingType::EBT_Pillar:
			return true;
			break;
		default:
			return false;
			break;
		}
	}
	else
	{
		FVector Start = BuildTransform.GetLocation();
		if (Buildables[BuildID]->BuildingType == EBuildingType::EBT_Foundation)
		{
			Start = Start + FVector(0.f, 0.f, 80.f);
		}
		FVector End = BuildTransform.GetLocation() - FVector(0.f, 0.f, BuildingSupportHeight);
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner());
		FHitResult HitResult;

		bool bHit = UKismetSystemLibrary::LineTraceSingle(BuildGhostComponent,
			Start,
			End,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::ForOneFrame,
			HitResult,
			true,
			FLinearColor::Yellow);

		if (bHit)
		{
			if (HitResult.GetActor()->IsA(Buildables[BuildID]->BuildingClass))
			{
				return false;
			}
			else return bHit;
		}
		return bHit;
	}
}

bool UBuildingComponent::IsBuildingColliding()
{
	FVector Origin;
	FVector BoxExtent;
	float SphereRadius;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());
	FHitResult HitResult;

	UKismetSystemLibrary::GetComponentBounds(BuildGhostComponent, Origin, BoxExtent, SphereRadius);

	switch (Buildables[BuildID]->BuildingType)
	{
	case EBuildingType::EBT_Foundation:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Pillar:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Wall:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Doorway:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_WindowWall:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Ceiling:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Ramp:
		BoxExtent = BoxExtent / 1.2f;
		break;
	case EBuildingType::EBT_Door:
		BoxExtent = BoxExtent / 1.1f;
		BoxExtent.Z = BoxExtent.Z - 10.f;
		break;
	case EBuildingType::EBT_Window:
		BoxExtent = BoxExtent / 1.05f;
		BoxExtent.Z = BoxExtent.Z - 10.f;
		break;
	}

	bool bHit = UKismetSystemLibrary::BoxTraceSingle(BuildGhostComponent,
		Origin,
		Origin,
		BoxExtent,
		FRotator(0.f, 0.f, 0.f),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true,
		FLinearColor::Blue);

	return bHit;
}

void UBuildingComponent::RotateBuildGhostMesh(float YawRotation)
{
	if (bIsBuildModeOn && BuildGhostComponent)
	{ 
		BuildTransform = FTransform(UKismetMathLibrary::ComposeRotators(
			BuildTransform.Rotator(), FRotator(0.f, YawRotation, 0.f)), BuildTransform.GetLocation(), BuildTransform.GetScale3D());
	}
}

void UBuildingComponent::ChangeBuildGhostMesh()
{
	if (BuildGhostComponent) BuildGhostComponent->SetStaticMesh(Buildables[BuildID]->Mesh);
}

void UBuildingComponent::SpawnBuilding_Server_Implementation(TSubclassOf<ABuildableBase> BuildingClass, const FTransform& Transform, AActor* Owner, APawn* Instigator)
{
	ABuildableBase* SpawnedBuilding = GetWorld()->SpawnActorDeferred<ABuildableBase>(
		BuildingClass,
		Transform,
		Owner,
		Instigator);

	UGameplayStatics::FinishSpawningActor(SpawnedBuilding, Transform);
}


