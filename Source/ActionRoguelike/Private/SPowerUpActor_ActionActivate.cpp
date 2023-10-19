// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor_ActionActivate.h"
#include "SAction.h"
#include "SActionComponent.h"

ASPowerUpActor_ActionActivate::ASPowerUpActor_ActionActivate()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
}

void ASPowerUpActor_ActionActivate::Interact_Implementation(APawn* InstigatorPawn)
{
	USActionComponent* ActionComp = USActionComponent::GetActions(InstigatorPawn);
	if (!ActionComp)
	{
		return;
	}

	if (ActionComp->HasAction(ActivateActionClass))
	{
		return;
	}

	if (PowerUp())
	{
		ActionComp->AddAction(InstigatorPawn, ActivateActionClass);
	}
}
