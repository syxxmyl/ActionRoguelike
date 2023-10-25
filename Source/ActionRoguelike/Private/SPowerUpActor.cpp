// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerUpActor::ASPowerUpActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	RootComponent = SphereComp;

	PowerUpCDTime = 10.0f;
	bIsVisible = true;

	bReplicates = true;
}

void ASPowerUpActor::ResetCD()
{
	bIsVisible = true;
	OnRep_VisibleChanged();
}

void ASPowerUpActor::OnRep_VisibleChanged()
{
	SetActorEnableCollision(bIsVisible);
	RootComponent->SetVisibility(bIsVisible, true);	
}

void ASPowerUpActor::Interact_Implementation(APawn* InstigatorPawn)
{
	
}

FText ASPowerUpActor::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	return FText::GetEmpty();
}

bool ASPowerUpActor::PowerUp()	
{
	if (bIsVisible)
	{
		bIsVisible = false;
		OnRep_VisibleChanged();
		GetWorldTimerManager().SetTimer(TimerHandle_CD, this, &ASPowerUpActor::ResetCD, PowerUpCDTime);
		return true;
	}

	return false;
}

void ASPowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerUpActor, bIsVisible);
}
