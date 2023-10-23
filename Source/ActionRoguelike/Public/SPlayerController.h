// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, APlayerState*, NewPlayerState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnChanged, APawn*, NewPawn);


class UUserWidget;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintBeginPlayingState();

	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();

	void SetupInputComponent() override;

	virtual void BeginPlayingState() override;

	virtual void SetPawn(APawn* InPawn) override;

	void OnRep_PlayerState() override;

protected:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChanged OnPlayerStateReceived;

	UPROPERTY(BlueprintAssignable)
	FOnPawnChanged OnPawnChanged;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY()
	UUserWidget* PauseMenuInstance;
};
