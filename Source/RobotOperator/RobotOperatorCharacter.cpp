// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RobotOperatorCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine.h"
#include "Runtime/Engine//Classes/Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// ARobotOperatorCharacter

ARobotOperatorCharacter::ARobotOperatorCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	Hitbox = GetCapsuleComponent();
	Hitbox->SetCollisionProfileName(TEXT("Collision Box"));
	Hitbox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Hitbox->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	Hitbox->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECR_Block);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->bAbsoluteRotation = true;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->bAbsoluteRotation = true;
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	DamageDealt = false;

	Health = 100;
}

//////////////////////////////////////////////////////////////////////////
// Animation

void ARobotOperatorCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	// Are we moving or standing still?
	UPaperFlipbook* DesiredAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	if( GetSprite()->GetFlipbook() != DesiredAnimation 	)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void ARobotOperatorCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	//Call function to move the player
	MoveRight();

	//Gets Player location 
	PlayerLocation = this->GetActorLocation();

	//Checks player location to see if player has reached the end of the level
	if (PlayerLocation.X >= LevelOneEnd.X)
	{
		//Turns Autorun off
		AutoRun = 0;

		APlayerController* MyController = GetWorld()->GetFirstPlayerController();

		MyController->bShowMouseCursor = true;
		MyController->bEnableClickEvents = true;
		MyController->bEnableMouseOverEvents = true;

	}
	UpdateCharacter();	

	// On
	Hitbox->OnComponentBeginOverlap.AddDynamic(this, &ARobotOperatorCharacter::OnBeginOverlap);

	FOutputDeviceNull ar;

	this->CallFunctionByNameWithArguments(TEXT("DisplayHUD"), ar, NULL, true);

}


//////////////////////////////////////////////////////////////////////////
// Input

void ARobotOperatorCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//PlayerInputComponent->BindAxis("MoveRight", this, &ARobotOperatorCharacter::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARobotOperatorCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARobotOperatorCharacter::TouchStopped);
}

void ARobotOperatorCharacter::MoveRight()
{
	// Makes the character constantly run (To stop runing set AutoRun to 0)
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AutoRun);
}

void ARobotOperatorCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void ARobotOperatorCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void ARobotOperatorCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}

void ARobotOperatorCharacter::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	// Once the Player as hit an Obstacle actor it will trigger the load level event
	if (OtherActor->ActorHasTag(FName(TEXT("Obstacle"))))
	{

		if (DamageDealt == false)
		{
			DamageDealt = true;

			Health -= 10;

			GetWorldTimerManager().SetTimer(TimerHandle, this, &ARobotOperatorCharacter::TookDamage, 0.001f, false);

			UE_LOG(LogTemp, Warning, TEXT("Damage"));
		}

		if (Health <= 0)
		{
			FOutputDeviceNull ar;

			this->CallFunctionByNameWithArguments(TEXT("DisplayHUD"), ar, NULL, true);

		}
	}

}

void ARobotOperatorCharacter::OnBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Once the Player as hit an Obstacle actor it will trigger the load level event
	if (OtherActor->ActorHasTag(FName(TEXT("Obstacle"))))
	{
		if (DamageDealt == false)
		{
			DamageDealt = true;

			Health -= 10;

			GetWorldTimerManager().SetTimer(TimerHandle, this, &ARobotOperatorCharacter::TookDamage, 0.001f, false);

			FOutputDeviceNull ar;

			this->CallFunctionByNameWithArguments(TEXT("DisplayHUD"), ar, NULL, true);

			UE_LOG(LogTemp, Warning, TEXT("Damage"));
		}

		if (Health <= 0)
		{
			FOutputDeviceNull ar2;
			this->CallFunctionByNameWithArguments(TEXT("RemoveViewports"), ar2, NULL, true);
			this->CallFunctionByNameWithArguments(TEXT("GameOver"), ar2, NULL, true);
			Destroy();
		}
	}
}

void ARobotOperatorCharacter::TookDamage()
{
	DamageDealt = false;
}
