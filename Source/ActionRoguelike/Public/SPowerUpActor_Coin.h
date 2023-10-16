// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPowerUpActor.h"
#include "SPowerUpActor_Coin.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPowerUpActor_Coin : public ASPowerUpActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASPowerUpActor_Coin();

	virtual void Interact_Implementation(APawn* InstigatorPawn) override;


protected:

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Credit")
	float ObtainCreditAmount;
};
