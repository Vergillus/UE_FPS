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

	den = 1.0f;
}


void AMyFPSHUD::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("HUD Begin Play!!!!!"));	

	
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
	const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTex->GetSizeX() / 2)*den), (Center.Y - (CrosshairTex->GetSizeY() / 2)*den));
	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FVector2D(CrosshairTex->GetSizeX() * den, CrosshairTex->GetSizeY() * den), FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
	//UE_LOG(LogTemp, Warning, TEXT("DEN: %f"), CharRef->SpreadRadius);	

	//for (int i = 0; i < Size; i++)
	//{
	//	//Draw2DLine(LOL[i%Size].X, LOL[i%Size].Y, LOL[(i + 1) % Size].X, LOL[(i + 1) % Size].Y,FColor::Red);

	//	//DrawLine(LOL[i%Size].X, LOL[i%Size].Y, LOL[(i + 1) % Size].X, LOL[(i + 1) % Size].Y, FColor::Red, 5.f);
	//}

	

	

}


