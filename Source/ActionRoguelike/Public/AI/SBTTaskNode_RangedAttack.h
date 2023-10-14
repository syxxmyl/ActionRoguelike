// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SBTTaskNode_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USBTTaskNode_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	USBTTaskNode_RangedAttack();

private:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxBulletSpread;
};
