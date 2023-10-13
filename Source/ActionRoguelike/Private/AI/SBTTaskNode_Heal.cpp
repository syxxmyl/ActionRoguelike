// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTTaskNode_Heal.h"
#include "AIController.h"
#include "AI/SAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SAttributeComponent.h"

EBTNodeResult::Type USBTTaskNode_Heal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (ensure(AIController))
	{
		ASAICharacter* AICharacter = Cast<ASAICharacter>(AIController->GetPawn());
		if (!AICharacter)
		{
			return EBTNodeResult::Failed;
		}
		
		if (ensure(AICharacter))
		{
			USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(AICharacter->GetComponentByClass(USAttributeComponent::StaticClass()));
			if (AttributeComp)
			{
				return AttributeComp->ApplyHealthChange(AttributeComp->GetHealthMax()) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
				
			}
		}
	}

	return EBTNodeResult::Failed;
}
