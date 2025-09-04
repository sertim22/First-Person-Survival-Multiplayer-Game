// Fill out your copyright notice in the Description page of Project Settings.


#include "HopeAssetManager.h"
#include "HopeGameplayTags.h"
#include "AbilitySystemGlobals.h"

UHopeAssetManager& UHopeAssetManager::Get()
{
	check(GEngine);

	UHopeAssetManager* HopeAssetManager = Cast<UHopeAssetManager>(GEngine->AssetManager);
	return *HopeAssetManager;
}

void UHopeAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FHopeGameplayTags::InitializeNativeGameplayTags();

	//Required to use Target Data
	UAbilitySystemGlobals::Get().InitGlobalData();
}
