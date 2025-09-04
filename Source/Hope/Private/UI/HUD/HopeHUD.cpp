// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/HopeHUD.h"
#include "Widgets/Inv_Widget.h"
#include "Blueprint/UserWidget.h"

void AHopeHUD::InitializeWidgets(APlayerController* PlayerController)
{
	// Inventory Widget
	check(Inv_WidgetClass);
	Inv_Widget = CreateWidget<UInv_Widget>(PlayerController, Inv_WidgetClass);
	if (Inv_Widget) Inv_Widget->AddToViewport();
}
