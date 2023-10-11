// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	Health = 100.0f;
	HealthMax = 100.0f;
}


bool USAttributeComponent::ApplyHealthChange(float Delta)
{
	Health += Delta;
	Health = FMath::Clamp(Health, 0, HealthMax);
	OnHealthChanged.Broadcast(nullptr, this, Health, Delta);
	return true;
}

bool USAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

