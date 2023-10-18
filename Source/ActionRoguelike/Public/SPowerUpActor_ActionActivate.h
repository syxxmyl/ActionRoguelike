// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPowerUpActor.h"
#include "SPowerUpActor_ActionActivate.generated.h"


class USAction;
class UStaticMeshComponent;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPowerUpActor_ActionActivate : public ASPowerUpActor
{
	GENERATED_BODY()
	
public:
	ASPowerUpActor_ActionActivate();

	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Action")
	TSubclassOf<USAction> ActivateActionClass;

	UPROPERTY(EditAnywhere, Category = "Component")
	UStaticMeshComponent* MeshComp;
};
