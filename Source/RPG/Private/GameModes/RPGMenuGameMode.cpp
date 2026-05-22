// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/RPGMenuGameMode.h"
#include "Controllers/RPGMenuPlayerController.h"

ARPGMenuGameMode::ARPGMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ARPGMenuPlayerController::StaticClass();
}
