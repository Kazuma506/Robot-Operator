// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "RobotOperatorCharacter.generated.h"

class UTextRenderComponent;

/**
 * This class is the default character for RobotOperator, and it is responsible for all
 * physical interaction between the player and the world.
 *
 * The capsule component (inherited from ACharacter) handles collision with the world
 * The CharacterMovementComponent (inherited from ACharacter) handles movement of the collision capsule
 * The Sprite component (inherited from APaperCharacter) handles the visuals
 */
UCLASS(config=Game)
class ARobotOperatorCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UTextRenderComponent* TextComponent;
	virtual void Tick(float DeltaSeconds) override;
protected:
	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animations)
	class UPaperFlipbook* RunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation();

	// Sets Player movement to constantly run
	void MoveRight();

	void UpdateCharacter();

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	void TookDamage();

	// Handles Player Input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Function to handle the events when the Player hits an obstacle
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	// Creates variable to control players speed
	float AutoRun = 1;

	// Creates a varaible for the Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health;

	// Creates variable to hold the score
	float Score;

	// Determines if the player is taking damage
	bool DamageDealt;

	// Creates TimerHandle for the timer
	FTimerHandle TimerHandle;

	// Location of the end of the level
	FVector LevelOneEnd = FVector(14788.0, -13.4, -445);

	// Location of Player
	FVector PlayerLocation = FVector(0,0,0);

	//Start position of Player
	FVector StartPosition = FVector(-2302, -13.4, -445);

	//Creates Hitbox for Player
	UPROPERTY(EditAnywhere, Category = "Hit Box")
		UCapsuleComponent* Hitbox;

public:
	ARobotOperatorCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
