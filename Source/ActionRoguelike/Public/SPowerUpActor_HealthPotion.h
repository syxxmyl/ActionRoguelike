// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPowerUpActor.h"
#include "SPowerUpActor_HealthPotion.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ASPowerUpActor_HealthPotion : public ASPowerUpActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUpActor_HealthPotion();

	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

	virtual FText GetInteractText_Implementation(APawn* InstigatorPawn) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Health")
	float AddHealthAmount;	

	UPROPERTY(EditAnywhere, Category = "Health|Cost")
	float ConsumeCreditAmount;
};
