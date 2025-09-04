// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HopeHUD.generated.h"

class UUserWidget;
class UInv_Widget;

// HUD class used by this project. Contains all Widget Classes and InitializeOverlay function.
UCLASS()
class HOPE_API AHopeHUD : public AHUD
{
	GENERATED_BODY()

public:

	void InitializeWidgets(APlayerController* PlayerController);

	UPROPERTY()
	TObjectPtr<UInv_Widget> Inv_Widget;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Overlay Widget")
	TSubclassOf<UInv_Widget> Inv_WidgetClass;
};