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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Health")
	float AddHealthAmount;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(APawn* InstigatorPawn) override;
};
