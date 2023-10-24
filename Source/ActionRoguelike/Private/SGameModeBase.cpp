// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "AI/SAICharacter.h"
#include "SAttributeComponent.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "SPlayerState.h"
#include "SPowerUpActor.h"
#include "SSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "SGameplayInterface.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "SMonsterData.h"
#include "../ActionRoguelike.h"
#include "SActionComponent.h"


static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("su.SpawnBots"), true, TEXT("Enable Spawning of Bots via Timer"), ECVF_Cheat);

ASGameModeBase::ASGameModeBase()
{
	SpawnBotTimerInterval = 2.0f;
	PlayerRespawnTimerInterval = 2.0f;
	KillMinionObtainCreditAmount = 20.0f;

	PowerUpActorAmount = 5.0f;
	SpawnPowerUpActorTimerInterval = 1.0f;

	SlotName = "SaveGame01";
}


void ASGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	LoadSaveGame();
}


void ASGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ASPlayerState* PlayerState = NewPlayer->GetPlayerState<ASPlayerState>();
	if (PlayerState)
	{
		PlayerState->LoadPlayerState(CurrentSaveGame);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ASGameModeBase::StartPlay()
{
	Super::StartPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnBotTimerInterval, true);

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnPowerUpActor, this, &ASGameModeBase::SpawnPowerUpActorTimerElapsed, SpawnPowerUpActorTimerInterval, false);

	LoadSaveActorData();
}

void ASGameModeBase::KillAll()
{
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Bot = *It;
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (AttributeComp && AttributeComp->GetHealth() > 0.0f)
		{
			AttributeComp->ApplyHealthChange(this, -AttributeComp->GetHealthMax());
		}
	}
}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		UE_LOG(LogTemp, Log, TEXT("Bots Spawning disable via cvar 'CVarSpawnBots'"));
		return;
	}

	int32 NearOFAliveBots = 0;
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Bot = *It;
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (AttributeComp && AttributeComp->GetHealth() > 0.0f)
		{
			++NearOFAliveBots;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NearOFAliveBots);

	float MaxBotCount = 10.0f;
	if (DifficultyCurve)
	{
		MaxBotCount = DifficultyCurve->GetFloatValue(GetWorld()->GetTimeSeconds());
	}

	if (NearOFAliveBots >= MaxBotCount)
	{
		UE_LOG(LogTemp, Log, TEXT("At maximum bot capacity. Skipping bot spawn."));
		return;
	}

	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);

	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnSpawnBotQueryCompleted);
	}
}

void ASGameModeBase::OnSpawnBotQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn Bot EQS Query Failed!"));
		return;
	}

	TArray<FVector>Locations = QueryInstance->GetResultsAsLocations();

	if (Locations.Num() > 0)
	{
		if (MonsterTable)
		{
			TArray<FMonsterInfoRow*> Rows;
			MonsterTable->GetAllRows("", Rows);

			int32 randomIdx = FMath::RandRange(0, Rows.Num() - 1);
			FMonsterInfoRow* SelectedRow = Rows[randomIdx];

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* NewBot = GetWorld()->SpawnActor<ASAICharacter>(SelectedRow->MonsterData->MonsterClass, Locations[0], FRotator::ZeroRotator, SpawnParams);
			if (NewBot)
			{
				LogOnScreen(this, FString::Printf(TEXT("Spawned enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(SelectedRow->MonsterData)));
				USActionComponent* ActionComp = USActionComponent::GetActions(NewBot);
				if (ActionComp)
				{
					for (TSubclassOf<USAction> ActionClass : SelectedRow->MonsterData->Actions)
					{
						ActionComp->AddAction(NewBot, ActionClass);
					}
				}

			}
		}

		DrawDebugSphere(GetWorld(), Locations[0], 50.0f, 20, FColor::Blue, false, 60.0f);
	}
}

void ASGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	if (Controller)
	{
		Controller->UnPossess();
		RestartPlayer(Controller);
	}
}

void ASGameModeBase::SpawnPowerUpActorTimerElapsed()
{
	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnPowerUpActorQuery, this, EEnvQueryRunMode::AllMatching, nullptr);

	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnSpawnPowerUpActorQueryCompleted);
	}
}

void ASGameModeBase::OnSpawnPowerUpActorQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn PowerUpActor EQS Query Failed!"));
		return;
	}

	TArray<FVector>Locations = QueryInstance->GetResultsAsLocations();

	int32 TotalLoopNum = FMath::Min(PowerUpActorAmount, Locations.Num());

	int32 TotalSpawnNum = 0;
	while (TotalSpawnNum < PowerUpActorAmount && Locations.Num() > 0)
	{
		int32 RandomLocationIdx = FMath::RandRange(0, Locations.Num() - 1);
		FVector SelectedLocation = Locations[RandomLocationIdx];
		Locations.RemoveAt(RandomLocationIdx);

		FActorSpawnParameters SpawnParams;
		int32 RandomActor = FMath::RandRange(0, PowerUpActorClasses.Num() - 1);
		ASPowerUpActor* SpawnActor = GetWorld()->SpawnActor<ASPowerUpActor>(PowerUpActorClasses[RandomActor], SelectedLocation, FRotator::ZeroRotator);
		if (SpawnActor)
		{
			++TotalSpawnNum;
		}
	}
}

void ASGameModeBase::TryToRespawnPlayer(AActor* VictimActor)
{
	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, PlayerRespawnTimerInterval, false);
	}
}

void ASGameModeBase::TryToAddCredits(AActor* VictimActor, AActor* Killer)
{
	ASCharacter* Player = Cast<ASCharacter>(Killer);
	if (!Player)
	{
		return;
	}

	ASAICharacter* MinionBot = Cast<ASAICharacter>(VictimActor);
	if (!MinionBot)
	{	
		return;
	}

	ASPlayerState* PlayerState = ASPlayerState::GetPlayerState(Player);
	if (!PlayerState)
	{
		return;
	}

	PlayerState->ApplyCreditChange(KillMinionObtainCreditAmount);
}

void ASGameModeBase::WriteSaveActorData()
{
	if (!CurrentSaveGame)
	{
		return;
	}

	CurrentSaveGame->SaveActors.Empty();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor->Implements<USGameplayInterface>())
		{
			continue;
		}

		FActorSaveData ActorSaveData;
		ActorSaveData.ActorName = Actor->GetName();
		ActorSaveData.Transform = Actor->GetTransform();

		FMemoryWriter MemWriter(ActorSaveData.ByteData);
		FObjectAndNameAsStringProxyArchive ArchiveData(MemWriter, true);
		ArchiveData.ArIsSaveGame = true;

		Actor->Serialize(ArchiveData);

		CurrentSaveGame->SaveActors.Add(ActorSaveData);
	}
}

void ASGameModeBase::LoadSaveActorData()
{
	if (!CurrentSaveGame || CurrentSaveGame->SaveActors.Num() == 0)
	{
		return;
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor->Implements<USGameplayInterface>())
		{
			continue;
		}

		for (FActorSaveData& ActorSaveData : CurrentSaveGame->SaveActors)
		{
			if (ActorSaveData.ActorName == Actor->GetName())
			{
				Actor->SetActorTransform(ActorSaveData.Transform);

				FMemoryReader MemReader(ActorSaveData.ByteData);
				FObjectAndNameAsStringProxyArchive ArchiveData(MemReader, true);
				ArchiveData.ArIsSaveGame = true;

				Actor->Serialize(ArchiveData);

				ISGameplayInterface::Execute_OnActorLoaded(Actor);

				break;
			}
		}
	}
}

void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	TryToRespawnPlayer(VictimActor);

	TryToAddCredits(VictimActor, Killer);

	

	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim is %s, Killer is %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
}

void ASGameModeBase::WriteSaveGame()
{
	for (int32 i = 0; i != GameState->PlayerArray.Num(); ++i)
	{
		ASPlayerState* PlayerState = Cast<ASPlayerState>(GameState->PlayerArray[i]);
		if (PlayerState)
		{
			PlayerState->SavePlayerState(CurrentSaveGame);
			break;
		}
	}

	WriteSaveActorData();

	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
}

void ASGameModeBase::LoadSaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (!CurrentSaveGame)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load SaveGame Data."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Loaded SaveGame Data."));
	}
	else
	{
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::CreateSaveGameObject(USSaveGame::StaticClass()));

		UE_LOG(LogTemp, Log, TEXT("Created New SaveGame Data."));
	}
}

