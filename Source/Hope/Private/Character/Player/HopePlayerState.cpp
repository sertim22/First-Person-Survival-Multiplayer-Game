// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/HopePlayerState.h"
#include "AbilitySystem/HopeAbilitySystemComponent.h"
#include "AbilitySystem/HopeAttributeSet.h"

AHopePlayerState::AHopePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UHopeAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UHopeAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AHopePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
