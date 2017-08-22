// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MyFPSCharacter.h"
#include "MyFPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "MyFPSHUD.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMyFPSCharacter

AMyFPSCharacter::AMyFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);	

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;		

	IsFiring = false;
	bCanZoom = false;
	bCanHook = false;
	FireCooldown = 0.0f;
	HookCooldown = 0.0f;
	SpreadRadius = 1.0f;
	ForceMulti = 10000.0f;	

}

void AMyFPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	MovementComp = AMyFPSCharacter::GetCharacterMovement();
	HUD = Cast<AMyFPSHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

}

void AMyFPSCharacter::Tick(float DeltaSeconds)
{		
	Super::Tick(DeltaSeconds);

	if (IsFiring)
	{
		//TO DO : Implement Timer in here
		FireCooldown -= 0.8f;
		if (FireCooldown <= 0.0f) 
		{
			SerialFire();
			SpreadRadius += 25.f * DeltaSeconds;			
			FireCooldown = 5.0f;
		}		
	}
	else{SpreadRadius -= 10.f * DeltaSeconds;}
	//UE_LOG(LogTemp, Warning, TEXT("Spread Radius: %f"), SpreadRadius);
	SpreadRadius = FMath::Clamp(SpreadRadius, 1.f, 300.f);

	if (bCanHook)
	{
		HookCooldown -= 0.5f;
		if (HookCooldown <= 0.0f)
		{			
			CalculateHook(HitRes);
			HookCooldown = 10.f;
			bCanHook = false;
						
		}
		
	}
	//UE_LOG(LogTemp, Warning, TEXT("Radius: %f"), SpreadRadius);
	
	if (HUD != nullptr)
	{
		HUD->ScaleMultiplayer = SpreadRadius;
		//UE_LOG(LogTemp, Warning, TEXT("%f"), HUD->den);
	}

	if (bCanZoom)
	{
		ZoomIn();
		StopFire();
	}
	else
	{
		ZoomOut();
	}	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyFPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("GrappleHook", IE_Pressed, this, &AMyFPSCharacter::SpreadRayCast);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AMyFPSCharacter::StartZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AMyFPSCharacter::StopZoom);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyFPSCharacter::TouchStarted);
	if (EnableTouchscreenMovement(PlayerInputComponent) == false)
	{		

		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyFPSCharacter::OnFire);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyFPSCharacter::StopFire);
		
	}	

	PlayerInputComponent->BindAction("ZoomFire", IE_Pressed, this, &AMyFPSCharacter::ZoomFire);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyFPSCharacter::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyFPSCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyFPSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyFPSCharacter::LookUpAtRate);
}

void AMyFPSCharacter::OnFire()
{
	if (!bCanZoom)
	{
		IsFiring = true;
	}
	
}

void AMyFPSCharacter::StopFire()
{
	IsFiring = false;
}

void AMyFPSCharacter::SerialFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AMyFPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				SpreadRayCast();				
				//const FRotator SpawnRotation = GetControlRotation();
				//// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				//const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				////Set Spawn Collision Handling Override
				//FActorSpawnParameters ActorSpawnParams;
				//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				//// spawn the projectile at the muzzle
				//World->SpawnActor<AMyFPSProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}

void AMyFPSCharacter::SpreadRayCast()
{
	UWorld* const World = GetWorld();
	if (World)
	{
		
		FVector RndPointInCone;
		if (IsFiring)
		{
			 RndPointInCone = FMath::VRandCone(FirstPersonCameraComponent->GetForwardVector(), SpreadRadius);
		}
		else
		{
			RndPointInCone.ZeroVector;
		}
		//UE_LOG(LogTemp, Warning, TEXT("COne! : %s"), *ConeLOL.ToString());
		FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
		FVector TraceEnd = (TraceStart + RndPointInCone * 100.0f) + (FirstPersonCameraComponent->GetForwardVector() * 5000.f);
		
		FCollisionQueryParams* QueryParams = new FCollisionQueryParams();
		QueryParams->AddIgnoredActor(this);
		QueryParams->bTraceComplex = true;

		if (World->LineTraceSingleByChannel(HitRes, TraceStart, TraceEnd, ECC_Visibility, *QueryParams))
		{			
			if (!IsFiring && !bCanZoom)
			{
				bCanHook = true;				
			}

			else
			{
				FVector HitPoint = HitRes.ImpactPoint;
				FRotator HitRotation = HitRes.ImpactNormal.Rotation();

				float LifeSpan = FMath::RandRange(0.0f, 5.f);
				
				UGameplayStatics::SpawnDecalAtLocation(World, BulletHole, BulletHoleSize, HitPoint, HitRotation, LifeSpan);								
				UGameplayStatics::SpawnEmitterAtLocation(World, BulletParticle, HitPoint, HitRotation, true);
			}
			
			//DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, true, -1.0f, 0, 2.0f);
		}
	}

	
}

void AMyFPSCharacter::StraithRayCast()
{

}

void AMyFPSCharacter::CalculateHook(FHitResult Hit)
{
	FVector HitPoint = Hit.ImpactPoint;
	FVector Distance = HitPoint - GetActorLocation();
	float DistLenth;
	FVector Dir;
	
	Distance.ToDirectionAndLength(Dir, DistLenth);
	FVector ForceVector = Dir * ForceMulti;
	if (HitPoint.Z < FirstPersonCameraComponent->GetComponentLocation().Z)
	{
		ForceVector.Z = 10000.f;		
	}	

	//UE_LOG(LogTemp, Warning, TEXT("Force Vector: %s"), *ForceVector.ToString());

	//UCharacterMovementComponent* MoveComp = AMyFPSCharacter::GetCharacterMovement();	
	if (MovementComp)
	{
		MovementComp->AddImpulse(ForceVector);
	}
	

	//SetActorLocation(Destination);
}

void AMyFPSCharacter::StartZoom()
{
	bCanZoom = true;	
}

void AMyFPSCharacter::StopZoom()
{
	bCanZoom = false;
}

void AMyFPSCharacter::ZoomIn()
{
	FirstPersonCameraComponent->FieldOfView = 20.f;
}

void AMyFPSCharacter::ZoomOut()
{
	FirstPersonCameraComponent->FieldOfView = 90.f;
}

void AMyFPSCharacter::ZoomFire()
{
	SerialFire();
}

void AMyFPSCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyFPSCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMyFPSCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AMyFPSCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AMyFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyFPSCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyFPSCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AMyFPSCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyFPSCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMyFPSCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMyFPSCharacter::TouchUpdate);
	}
	return bResult;
}
