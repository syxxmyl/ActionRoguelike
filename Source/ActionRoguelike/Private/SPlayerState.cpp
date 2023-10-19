// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"
#include "Net/UnrealNetwork.h"

ASPlayerState::ASPlayerState()
{
	Credit = 0.0f;
}

bool ASPlayerState::ApplyCreditChange(float Delta)
{
	if (!CheckEnoughCredit(Delta))
	{
		return false;
	}

	if (Delta == 0.0f)
	{
		return false;
	}

	Credit += Delta;
	OnCreditChanged.Broadcast(this, Credit, Delta);

	UE_LOG(LogTemp, Log, TEXT("ApplyCreditChange: Credit is %f, Delta is %f"), Credit, Delta);
	return true;
}

bool ASPlayerState::CheckEnoughCredit(float Delta)
{
	if (Credit + Delta < 0.0f)
	{
		return false;
	}

	return true;
}

ASPlayerState* ASPlayerState::GetPlayerState(APawn* FromActor)
{
	if (FromActor)
	{
		return Cast<ASPlayerState>(FromActor->GetPlayerState());
	}

	return nullptr;
}

void ASPlayerState::OnRep_CreditChanged(float OldCredit)
{
	UE_LOG(LogTemp, Log, TEXT("Client OnRep_CreditChanged: Credit is %f, Delta is %f"), Credit, Credit - OldCredit);
	OnCreditChanged.Broadcast(this, Credit, Credit - OldCredit);
}

void ASPlayerState::ServerCreditChanged_Implementation(float Delta)
{	ApplyCreditChange(Delta);
}

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, Credit);
}
