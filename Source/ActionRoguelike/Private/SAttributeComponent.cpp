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
	float OldHealth = Health;
	Health = FMath::Clamp(Health + Delta, 0.0f, HealthMax);
	float RealDelta = Health - OldHealth;
	OnHealthChanged.Broadcast(nullptr, this, Health, RealDelta);
	return RealDelta != 0;
}

bool USAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

float USAttributeComponent::GetHealth() const
{
	return Health;
}

