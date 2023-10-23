// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"
#include "Net/UnrealNetwork.h"
#include "../ActionRoguelike.h"

void USAction::Initialize(USActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}

void USAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running: %s."), *GetNameSafe(this));
	// LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->ActiveGameplayTags.AppendTags(GrantsTags);
	}

	ActionRepData.bIsRunning = true;
	ActionRepData.InstigatorActor = Instigator;

	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}

void USAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped: %s."), *GetNameSafe(this));
	// LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->ActiveGameplayTags.RemoveTags(GrantsTags);
	}

	ActionRepData.bIsRunning = false;
	ActionRepData.InstigatorActor = Instigator;

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}

bool USAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
		{
			return false;
		}
	}

	return true;
}

bool USAction::CanStop_Implementation(AActor* Instigator)
{
	if (!IsRunning())
	{
		return false;
	}

	return true;
}

bool USAction::IsRunning() const
{
	return ActionRepData.bIsRunning;
}

UWorld* USAction::GetWorld() const
{
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}
	
	return nullptr;
}

USActionComponent* USAction::GetOwningComponent() const
{
	return ActionComp;
}

void USAction::OnRep_ActionRepDataChanged()
{
	if (ActionRepData.bIsRunning)
	{
		StartAction(ActionRepData.InstigatorActor);
	}
	else
	{
		StopAction(ActionRepData.InstigatorActor);
	}
}

bool USAction::IsSupportedForNetworking() const
{
	return true;
}

void USAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAction, ActionRepData);
	DOREPLIFETIME(USAction, ActionComp);
	DOREPLIFETIME(USAction, TimeStarted);
}