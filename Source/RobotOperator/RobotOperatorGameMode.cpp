// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RobotOperatorGameMode.h"
#include "RobotOperatorCharacter.h"

ARobotOperatorGameMode::ARobotOperatorGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = ARobotOperatorCharacter::StaticClass();	
}
