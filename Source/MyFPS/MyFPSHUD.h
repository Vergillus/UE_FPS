// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyFPSHUD.generated.h"

//class MyFPSCharacter;

UCLASS()
class AMyFPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMyFPSHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	virtual void BeginPlay();

	UPROPERTY()
	float den;	

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	UPROPERTY()
	int Size;
	
	UPROPERTY()
	float Angle;

	UPROPERTY()
	float Slice;

	UPROPERTY()
	TArray<FVector2D> LOL;

};

