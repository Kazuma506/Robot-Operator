// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RobotOperatorController.generated.h"

/**
 * 
 */
UCLASS()
class ROBOTOPERATOR_API ARobotOperatorController : public AAIController
{
	GENERATED_BODY()
	
	
public: 
	ARobotOperatorController();

	virtual void Tick(float DeltaTime) override;
	
	 FVector LevelOneEnd = FVector(2338, -13.4, -445);
};
