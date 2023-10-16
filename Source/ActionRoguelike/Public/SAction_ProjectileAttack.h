// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SAction.h"
#include "SAction_ProjectileAttack.generated.h"


class ASProjectileBase;
class UParticleSystem;
class UAnimMontage;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USAction_ProjectileAttack : public USAction
{
	GENERATED_BODY()
	
public:

	USAction_ProjectileAttack();

	virtual void StartAction_Implementation(AActor* InstigatorActor) override;


protected:

	UFUNCTION()
	void AttackDelay_Elapsed(ACharacter* InstigatorCharacter);

protected:

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<ASProjectileBase> ClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UParticleSystem* AttachedEffect;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName HandLocationSocketName;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* PrimaryAttackAnim;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float ProjectileSpawnDelayTime;
};
