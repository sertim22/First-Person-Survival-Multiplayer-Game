// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"

/**
 * Hope Gameplay Tags
 *
 * Singleton containing native Gameplay Tags
 */

struct FHopeGameplayTags
{

public:

	static const FHopeGameplayTags& Get() { return GameplayTags; }
	// Add new GameplayTags to this function so they are visible in the editor and C++
	static void InitializeNativeGameplayTags();

	// Gameplay Tags
	

	/*
	* Abilities
	*/
	FGameplayTag Abilities_None;
	/*
	* Abilities end
	*/


	/*
	* Sockets
	*/
	FGameplayTag CombatSocket_ArrowSocket;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	/*
	* Sockets end
	*/


	/*
	* ALS
	*/
	FGameplayTag Gameplay_MovementStopped;
	/*
	* ALS end
	*/

private:

	static FHopeGameplayTags GameplayTags;
};