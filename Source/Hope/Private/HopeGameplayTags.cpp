// Fill out your copyright notice in the Description page of Project Settings.


#include "HopeGameplayTags.h"
#include "GameplayTagsManager.h"

FHopeGameplayTags FHopeGameplayTags::GameplayTags;

void FHopeGameplayTags::InitializeNativeGameplayTags()
{
	/*
	* Abilities
	*/
	GameplayTags.Abilities_None = UGameplayTagsManager::Get().AddNativeGameplayTag("Abilities.None", FString("No Ability - nullptr for Ability Tags"));
	/*
	* Abilities end
	*/


	/*
	* Sockets
	*/
	GameplayTags.CombatSocket_ArrowSocket = UGameplayTagsManager::Get().AddNativeGameplayTag("CombatSocket.ArrowSocket", FString("ArrowSocket"));
	GameplayTags.CombatSocket_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag("CombatSocket.RightHand", FString("RightHand"));
	GameplayTags.CombatSocket_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag("CombatSocket.LeftHand", FString("LeftHand"));
	/*
	* Sockets end
	*/


	/*
	* ALS 
	*/
	GameplayTags.Gameplay_MovementStopped = UGameplayTagsManager::Get().AddNativeGameplayTag("Gameplay.MovementStopped", FString("MovementStopped"));
	/*
	* ALS end
	*/
}
