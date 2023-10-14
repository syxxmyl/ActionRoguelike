// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerUpActor_HealthPotion.h"
#include "SCharacter.h"
#include "SAttributeComponent.h"

// Sets default values
ASPowerUpActor_HealthPotion::ASPowerUpActor_HealthPotion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	AddHealthAmount = 40.0f;

}

// Called when the game starts or when spawned
void ASPowerUpActor_HealthPotion::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASPowerUpActor_HealthPotion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASPowerUpActor_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	ASCharacter* GamePlayer = Cast<ASCharacter>(InstigatorPawn);
	if (!GamePlayer)
	{
		return;
	}

	USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(GamePlayer->GetComponentByClass(USAttributeComponent::StaticClass()));
	if (!AttributeComp)
	{
		return;
	}

	if (AttributeComp->GetHealthMax() == AttributeComp->GetHealth())
	{
		return;
	}

	if (PowerUp())
	{
		AttributeComp->ApplyHealthChange(this, AddHealthAmount);
	}
}

