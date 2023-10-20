// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"
#include "../Public/SGameModeBase.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier For Attribute component"), ECVF_Cheat);

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	Health = 100.0f;
	HealthMax = 100.0f;
	Rage = 0.0f;
	RageMax = 50.0f;

	SetIsReplicatedByDefault(true);
}

bool USAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// GodMode Cheat
	if (!GetOwner()->CanBeDamaged() && Delta < 0.0f)
	{
		return false;
	}

	float AmountMultiplier = 1.0f;
	if (Delta < 0.0f)
	{
		AmountMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
	}
	Delta = Delta * AmountMultiplier;

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, HealthMax);
	float RealDelta = NewHealth - OldHealth;

	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;
		if (RealDelta != 0.0f)
		{
			MulticastHealthChanged(InstigatorActor, Health, RealDelta);
		}

		if (RealDelta < 0.0f && Health == 0.0f)
		{
			ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
			if (GM)
			{
				GM->OnActorKilled(GetOwner(), InstigatorActor);
			}
		}
	}

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

USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<USAttributeComponent>(FromActor->GetComponentByClass(USAttributeComponent::StaticClass()));
	}

	return nullptr;
}

bool USAttributeComponent::IsActorAlive(AActor* Actor)
{
	USAttributeComponent* AttributeComp = GetAttributes(Actor);
	if (AttributeComp)
	{
		return AttributeComp->GetHealth() > 0.0f;
	}

	return false;
}

float USAttributeComponent::GetRageMax() const
{
	return RageMax;
}

float USAttributeComponent::GetRage() const
{
	return Rage;
}

bool USAttributeComponent::ApplyRageChange(AActor* InstigatorActor, float Delta)
{
	if (Rage + Delta < 0.0f)
	{
		return false;
	}

	float OldRage = Rage;
	float NewRage = FMath::Clamp(Rage + Delta, 0.0f, RageMax);
	float RealDelta = NewRage - OldRage;

	if (GetOwner()->HasAuthority())
	{
		Rage = NewRage;

		if (RealDelta != 0.0f)
		{
			MulticastRageChanged(InstigatorActor, Rage, RealDelta);
		}
	}
	
	return RealDelta != 0;
}

void USAttributeComponent::MulticastRageChanged_Implementation(AActor* InstigatorActor, float NewRage, float Delta)
{
	OnRageChanged.Broadcast(InstigatorActor, this, NewRage, Delta);
}

void USAttributeComponent::MulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}

void USAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAttributeComponent, Health);
	DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_InitialOnly);

	DOREPLIFETIME_CONDITION(USAttributeComponent, Rage, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USAttributeComponent, RageMax, COND_InitialOnly);
}
