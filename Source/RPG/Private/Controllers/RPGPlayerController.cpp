// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/RPGPlayerController.h"

ARPGPlayerController::ARPGPlayerController()
{
	PlayerTeamId = FGenericTeamId(0);
}

FGenericTeamId ARPGPlayerController::GetGenericTeamId() const
{
	return PlayerTeamId;
}
