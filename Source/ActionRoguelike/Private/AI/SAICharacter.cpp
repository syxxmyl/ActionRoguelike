// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "BrainComponent.h"
#include "SWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SActionComponent.h"


// Sets default values
ASAICharacter::ASAICharacter()
{
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

    AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

    ActionComp = CreateDefaultSubobject<USActionComponent>("ActionComp");

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    HitFlashParamName = "LastHitTime";

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
    GetMesh()->SetGenerateOverlapEvents(true);
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

        if (!ActiveHealthBar)
        {
			ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			if (ActiveHealthBar)
			{
                ActiveHealthBar->BindAttachedActor(this);
				ActiveHealthBar->AddToViewport();
			}
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

            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GetCharacterMovement()->DisableMovement();

            // set LifeSpan
            SetLifeSpan(10.0f);
        }
    }
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
    if (CurrentTargetActor != Pawn)
    {
		USWorldUserWidget* PlayerSpottedWidget = CreateWidget<USWorldUserWidget>(GetWorld(), PlayerSpottedWidgetClass);
		if (PlayerSpottedWidget)
		{
			PlayerSpottedWidget->BindAttachedActor(this);
			PlayerSpottedWidget->AddToViewport();
		}
    }

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

    CurrentTargetActor = TargetActor;
}

