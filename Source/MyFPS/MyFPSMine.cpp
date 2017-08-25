// Fill out your copyright notice in the Description page of Project Settings.

#include "MyFPSMine.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AMyFPSMine::AMyFPSMine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->InitBoxExtent(FVector(35.f, 30.f, 15.f));
	CollisionBox->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionBox->OnComponentHit.AddDynamic(this, &AMyFPSMine::OnHit);

	RootComponent = CollisionBox;

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	MoveComp->UpdatedComponent = CollisionBox;
	MoveComp->InitialSpeed = 2000.f;
	MoveComp->MaxSpeed = 2000.f;
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bShouldBounce = false;

	UpperBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UpperBody"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> UpperBodyObj(TEXT("StaticMesh'/Game/SpiderMine/mine_UpperBody_001.mine_UpperBody_001'"));
	if (UpperBodyObj.Succeeded())
	{
		UpperBody->SetStaticMesh(UpperBodyObj.Object);
	}
	UpperBody->SetRelativeScale3D(FVector(.15f, .15f, .15f));
	UpperBody->BodyInstance.SetCollisionProfileName(TEXT("NoCollision"));
	UpperBody->AttachToComponent(CollisionBox, FAttachmentTransformRules::KeepRelativeTransform);


	LowerBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LowerBody"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LowerBodyObj(TEXT("StaticMesh'/Game/SpiderMine/mine_Legs_001.mine_Legs_001'"));
	if (LowerBodyObj.Succeeded())
	{
		LowerBody->SetStaticMesh(LowerBodyObj.Object);
	}
	LowerBody->SetRelativeScale3D(FVector(.15f, .15f, .15f));
	LowerBody->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	LowerBody->BodyInstance.SetCollisionProfileName(TEXT("NoCollision"));
	LowerBody->AttachToComponent(CollisionBox, FAttachmentTransformRules::KeepRelativeTransform);

	InitialLifeSpan = 25.f;
	RotationSpeed = 3.f;

	bCanRotate = false;
}

// Called when the game starts or when spawned
void AMyFPSMine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyFPSMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	

	if (bCanRotate)
	{
		UpperBody->AddWorldRotation(FQuat(this->GetActorUpVector(), RotationSpeed * DeltaTime));
	}
	
}

void AMyFPSMine::OnHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if (OtherActor != nullptr && OtherActor != this && OtherComp != nullptr)
	{
		bCanRotate = true;		

		float Angle = (FMath::Acos(FVector::DotProduct(GetActorUpVector(), Hit.ImpactNormal)) * 180) / PI;
		FVector RotationDirection = FVector::CrossProduct(GetActorUpVector(), Hit.ImpactNormal) * Angle;

		UE_LOG(LogTemp, Warning, TEXT("Impact Normal: %s"), *RotationDirection.ToString());

		SetActorRotation(RotationDirection.Rotation());				
		CollisionBox->BodyInstance.SetCollisionProfileName(TEXT("OverlapAll"));
		
	}
}

