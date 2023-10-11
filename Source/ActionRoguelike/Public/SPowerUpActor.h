// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGameplayInterface.h"
#include "SPowerUpActor.generated.h"


class USphereComponent;

UCLASS(ABSTRACT)
class ACTIONROGUELIKE_API ASPowerUpActor : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerUpActor();

protected:

	void ResetCD();

	UPROPERTY(EditAnywhere)
	USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere, Category = "PowerUp")
	float PowerUpCDTime;

	bool IsInCD;

	FTimerHandle TimerHandle_CD;

public:
	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

	UFUNCTION(BlueprintCallable)
	bool PowerUp();
};
