// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "BrainComponent.h"


// Sets default values
ASAICharacter::ASAICharacter()
{
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

    AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    HitFlashParamName = "LastHitTime";
}

void ASAICharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAICharacter::OnPawnSeen);
    AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
}

void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
    if (Delta < 0.0f)
    {
        GetMesh()->SetScalarParameterValueOnMaterials(HitFlashParamName, GetWorld()->GetTimeSeconds());

        if (InstigatorActor && InstigatorActor != this)
        {
            SetTargetActor(InstigatorActor);
        }

        if (NewHealth <= 0.0f)
        {
            // stop BT
            AAIController* AIController = Cast<AAIController>(GetController());
            if (AIController)
            {
                AIController->GetBrainComponent()->StopLogic("Killed");
            }

            // set ragdoll
            GetMesh()->SetAllBodiesSimulatePhysics(true);
            GetMesh()->SetCollisionProfileName("Ragdoll");

            // set LifeSpan
            SetLifeSpan(10.0f);
        }
    }
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
    SetTargetActor(Pawn);
    DrawDebugString(GetWorld(), GetActorLocation(), "SET TARGET PLAYER", nullptr, FColor::White, 4.0f, true);
}

void ASAICharacter::SetTargetActor(AActor* TargetActor)
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->GetBlackboardComponent()->SetValueAsObject("TargetActor", TargetActor);
	}
}

