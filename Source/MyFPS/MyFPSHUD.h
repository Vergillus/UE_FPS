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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float ScaleMultiplayer;	

	void SetBulletText(const int BulletCount, const int MaxBullet);

	void SetCooldownText(float val);

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	class UTexture2D* ZoomCrosshairTex;

	bool bIsZooming;	

	UPROPERTY()
	int Size;
	
	UPROPERTY()
	float Angle;

	UPROPERTY()
	float Slice;

	UPROPERTY()
	TArray<FVector2D> LOL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FString BulletText;

	FString MaxBulletText;

	UPROPERTY()
	FString CooldownText;

};

