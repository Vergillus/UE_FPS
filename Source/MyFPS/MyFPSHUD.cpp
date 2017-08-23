// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MyFPSHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "MyFPSCharacter.h"

AMyFPSHUD::AMyFPSHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;	

	static ConstructorHelpers::FObjectFinder<UTexture2D> ZoomCrosshairTexObj(TEXT("Texture2D'/Game/StarterContent/Textures/T_Tech_Dot_M.T_Tech_Dot_M'"));
	ZoomCrosshairTex = ZoomCrosshairTexObj.Object;

	ScaleMultiplayer = 1.0f;
	bIsZooming = true;

}


void AMyFPSHUD::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("HUD Begin Play!!!!!"));	

	
	/*Size = 12;

	Slice = PI * 2 / (float)Size;
	FVector2D CircleCenter(Canvas->ClipX / 2, Canvas->ClipY / 2);

	for (int i = 0; i < Size; i++)
	{
		float Angle = Slice * i;
		float Radius = 100.f;

		float X = CircleCenter.X + FMath::Cos(Angle) * Radius;
		float Y = CircleCenter.Y + FMath::Sin(Angle) * Radius;

		LOL.Add(FVector2D(X, Y));

	}*/
}

void AMyFPSHUD::SetBulletText(const int BulletCount, const int MaxBullet)
{
	BulletText = FString::FromInt(BulletCount);
	MaxBulletText = FString("|").Append(FString::FromInt(MaxBullet));
}

void AMyFPSHUD::SetCooldownText(float val)
{
	CooldownText = FString::SanitizeFloat(FMath::CeilToFloat(val));
}


void AMyFPSHUD::DrawHUD()
{
	Super::DrawHUD();	
	// Draw very simple crosshair

	
	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	/*const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y + 20.0f));*/
	
	//const AMyFPSCharacter* CharRef = NewObject<AMyFPSCharacter>();
	
	//den = CharRef->SpreadRadius;
	
	const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTex->GetSizeX() / 2)*ScaleMultiplayer), (Center.Y - (CrosshairTex->GetSizeY() / 2)*ScaleMultiplayer));
	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FVector2D(CrosshairTex->GetSizeX() * ScaleMultiplayer, CrosshairTex->GetSizeY() * ScaleMultiplayer), FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);

	DrawRect(FColor::Red, 0, 0, (Canvas->ClipX / 2) / ScaleMultiplayer, 10);	
	DrawRect(FColor::Red, 1000.f, 500.f, 50.f, 50.f);
	DrawText(CooldownText, FLinearColor::Green, 1000.f, 500.f, 0, 2);

	DrawText(BulletText, FLinearColor::Yellow, 1000.f, 600.f, 0, 2);
	DrawText(MaxBulletText, FLinearColor::Yellow, 1025.f, 600.f, 0, 2);
	
	
	
	//UE_LOG(LogTemp, Warning, TEXT("DEN: %f"), CharRef->SpreadRadius);	

	//for (int i = 0; i < Size; i++)
	//{
	//	//Draw2DLine(LOL[i%Size].X, LOL[i%Size].Y, LOL[(i + 1) % Size].X, LOL[(i + 1) % Size].Y,FColor::Red);

	//	//DrawLine(LOL[i%Size].X, LOL[i%Size].Y, LOL[(i + 1) % Size].X, LOL[(i + 1) % Size].Y, FColor::Red, 5.f);
	//}

	

	

}


