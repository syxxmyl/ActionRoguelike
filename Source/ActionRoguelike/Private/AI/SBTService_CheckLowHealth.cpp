// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTService_CheckLowHealth.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/SAICharacter.h"
#include "SAttributeComponent.h"

void USBTService_CheckLowHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if (ensure(BlackBoardComp))
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (ensure(AIController))
		{
			ASAICharacter* AICharacter = Cast<ASAICharacter>(AIController->GetPawn());
			if (ensure(AICharacter))
			{
				USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(AICharacter);
				if (AttributeComp)
				{
					float HealthScale = AttributeComp->GetHealth() / AttributeComp->GetHealthMax();
					bool bLowHealth = HealthScale < LowerHealthScaleValue;
					BlackBoardComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
				}
			}
		}
		
	}
}
