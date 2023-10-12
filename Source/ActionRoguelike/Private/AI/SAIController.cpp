// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void ASAIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);

	APawn* GamePlayer = UGameplayStatics::GetPlayerPawn(this, 0);
	if (GamePlayer)
	{
		GetBlackboardComponent()->SetValueAsVector("MoveToLocation", GamePlayer->GetActorLocation());
		GetBlackboardComponent()->SetValueAsObject("TargetActor", GamePlayer);
	}
}
