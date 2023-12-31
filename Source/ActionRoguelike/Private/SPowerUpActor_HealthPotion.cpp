// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor_HealthPotion.h"
#include "SCharacter.h"
#include "SAttributeComponent.h"
#include "SPlayerState.h"


#define LOCTEXT_NAMESPACE "InteractableActors"

// Sets default values
ASPowerUpActor_HealthPotion::ASPowerUpActor_HealthPotion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	AddHealthAmount = 40.0f;
	ConsumeCreditAmount = 20.0f;
}

void ASPowerUpActor_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	ASCharacter* GamePlayer = Cast<ASCharacter>(InstigatorPawn);
	if (!GamePlayer)
	{
		return;
	}

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(GamePlayer);
	if (!AttributeComp)
	{
		return;
	}

	if (AttributeComp->GetHealthMax() == AttributeComp->GetHealth())
	{
		return;
	}

	ASPlayerState* PlayerState = ASPlayerState::GetPlayerState(GamePlayer);
	if (!PlayerState || !PlayerState->CheckEnoughCredit(-ConsumeCreditAmount))
	{
		return;
	}

	if (PowerUp())
	{
		PlayerState->ApplyCreditChange(-ConsumeCreditAmount);
		AttributeComp->ApplyHealthChange(this, AddHealthAmount);
	}
}

FText ASPowerUpActor_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
	if (AttributeComp && AttributeComp->GetHealthMax() == AttributeComp->GetHealth())
	{
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health.");
	}

	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} credits. Restore {1} health."), ConsumeCreditAmount, AddHealthAmount);
}

#undef LOCTEXT_NAMESPACE