// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "HopeAssetManager.generated.h"

// Custom asset manager class used to Initialize HopeGameplayTags
UCLASS()
class HOPE_API UHopeAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:

	static UHopeAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
