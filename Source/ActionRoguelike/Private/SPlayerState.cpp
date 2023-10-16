// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"

ASPlayerState::ASPlayerState()
{
	Credit = 0.0f;
}

bool ASPlayerState::ApplyCreditChange(float Delta)
{
	if (Credit + Delta < 0.0f)
	{
		return false;
	}

	Credit += Delta;
	OnCreditChanged.Broadcast(this, Credit, Delta);

	return true;
}
