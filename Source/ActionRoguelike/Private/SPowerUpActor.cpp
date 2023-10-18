// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "Components/SphereComponent.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	RootComponent = SphereComp;

	PowerUpCDTime = 10.0f;
	IsInCD = false;

	SetReplicates(true);
}

void ASPowerUpActor::ResetCD()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_CD);
	IsInCD = false;

	SetActorEnableCollision(true);
	RootComponent->SetVisibility(true, true);
}

void ASPowerUpActor::Interact_Implementation(APawn* InstigatorPawn)
{
	
}

bool ASPowerUpActor::PowerUp()	
{
	if (!IsInCD)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_CD, this, &ASPowerUpActor::ResetCD, PowerUpCDTime);
		IsInCD = true;

		SetActorEnableCollision(false);
		RootComponent->SetVisibility(false, true);
		return true;
	}

	return false;
}
