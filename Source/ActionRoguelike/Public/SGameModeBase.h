// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "SGameModeBase.generated.h"


class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;
class ASAICharacter;
class UCurveFloat;
class ASPowerUpActor;
class USSaveGame;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ASGameModeBase();

	virtual void StartPlay() override;

	UFUNCTION(Exec)
	void KillAll();

	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void WriteSaveGame();

	void LoadSaveGame();

	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

protected:

	UFUNCTION()
	void SpawnBotTimerElapsed();

	UFUNCTION()
	void OnSpawnBotQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller);

	UFUNCTION()
	void SpawnPowerUpActorTimerElapsed();

	UFUNCTION()
	void OnSpawnPowerUpActorQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	void TryToRespawnPlayer(AActor* VictimActor);

	void TryToAddCredits(AActor* VictimActor, AActor* Killer);

	void LoadSaveActorData();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UEnvQuery* SpawnBotQuery;

	FTimerHandle TimerHandle_SpawnBot;

	UPROPERTY(EditAnywhere, Category = "AI")
	float SpawnBotTimerInterval;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<ASAICharacter> MinionClass;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UCurveFloat* DifficultyCurve;

	UPROPERTY(EditAnywhere, Category = "Respawn")
	float PlayerRespawnTimerInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Credit")
	float KillMinionObtainCreditAmount;

	FTimerHandle TimerHandle_SpawnPowerUpActor;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUpActor")
	UEnvQuery* SpawnPowerUpActorQuery;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUpActor")
	float PowerUpActorAmount;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUpActor")
	TArray<TSubclassOf<ASPowerUpActor>> PowerUpActorClasses;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUpActor")
	float SpawnPowerUpActorTimerInterval;

	UPROPERTY()
	USSaveGame* CurrentSaveGame;

	FString SlotName;
};
