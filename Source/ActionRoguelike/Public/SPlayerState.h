// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditChanged, ASPlayerState*, OwningPlayerState, float, NewCredit, float, Delta);


class USSaveGame;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ASPlayerState();

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool ApplyCreditChange(float Delta);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool CheckEnoughCredit(float Delta);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	static ASPlayerState* GetPlayerState(APawn* FromActor);

	UFUNCTION(Server, Reliable)
	void ServerCreditChanged(float Delta);

	UFUNCTION(BlueprintNativeEvent)
	void SavePlayerState(USSaveGame* SaveObject);

	UFUNCTION(BlueprintNativeEvent)
	void LoadPlayerState(USSaveGame* SaveObject);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	float GetCredit();

protected:
	UFUNCTION()
	void OnRep_CreditChanged(float OldCredit);

protected:
	UPROPERTY(VisibleAnyWhere, ReplicatedUsing = "OnRep_CreditChanged", Category = "PlayerState")
	float Credit;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FOnCreditChanged OnCreditChanged;
};
