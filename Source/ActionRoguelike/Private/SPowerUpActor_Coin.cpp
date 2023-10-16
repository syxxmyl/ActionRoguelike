// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor_Coin.h"
#include "SCharacter.h"
#include "SAttributeComponent.h"
#include "SPlayerState.h"


ASPowerUpActor_Coin::ASPowerUpActor_Coin()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	ObtainCreditAmount = 20.0f;
}

void ASPowerUpActor_Coin::Interact_Implementation(APawn* InstigatorPawn)
{
	ASCharacter* GamePlayer = Cast<ASCharacter>(InstigatorPawn);
	if (!GamePlayer)
	{
		return;
	}

	ASPlayerState* PlayerState = ASPlayerState::GetPlayerState(GamePlayer);
	if (!PlayerState)
	{
		return;
	}

	if (PowerUp())
	{
		PlayerState->ApplyCreditChange(ObtainCreditAmount);
	}
}
