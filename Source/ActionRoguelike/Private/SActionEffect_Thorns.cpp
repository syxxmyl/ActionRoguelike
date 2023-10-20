// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionEffect_Thorns.h"
#include "SActionComponent.h"
#include "SAttributeComponent.h"


USActionEffect_Thorns::USActionEffect_Thorns()
{
	Duration = 0.0f;
	ThornsDamageWeight = 0.1f;
}

void USActionEffect_Thorns::StartAction_Implementation(AActor* Instigator)
{
	if (ActionComp)
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(ActionComp->GetOwner());
		if (AttributeComp)
		{
			AttributeComp->OnHealthChanged.AddDynamic(this, &USActionEffect_Thorns::OnHealthChanged);
		}
	}
}

void USActionEffect_Thorns::StopAction_Implementation(AActor* Instigator)
{
	if (ActionComp)
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(ActionComp->GetOwner());
		if (AttributeComp)
		{
			AttributeComp->OnHealthChanged.RemoveDynamic(this, &USActionEffect_Thorns::OnHealthChanged);
		}
	}
}

void USActionEffect_Thorns::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta >= 0.0f)
	{
		return;
	}

	if (!ActionComp)
	{
		return;
	}
	AActor* OwningActor = ActionComp->GetOwner();
	if (!OwningActor || OwningActor == InstigatorActor)
	{
		return;
	}

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorActor);
	if (AttributeComp)
	{
		int32 ThornDamage = FMath::RoundToInt(Delta * ThornsDamageWeight);
		if (ThornDamage == 0)
		{
			return;
		}

		AttributeComp->ApplyHealthChange(OwningActor, ThornDamage);
		UE_LOG(LogTemp, Log, TEXT("%s make %d ThornDamage by %f Original Damage"), *GetNameSafe(ActionComp->GetOwner()), ThornDamage, Delta);
	}
}
