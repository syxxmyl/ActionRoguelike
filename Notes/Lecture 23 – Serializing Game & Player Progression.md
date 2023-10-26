# Lecture 23 – Serializing Game & Player Progression

## 保存和读取游戏

新建一个 `USSaveGame `类继承自` USaveGame `类



### 在PlayerState里写好保存和读取函数

下面`GameModeBase`读取和保存的时候就直接调用每个Player的`PlayerState`的`Load/Save `函数就行了

```cpp
UFUNCTION(BlueprintNativeEvent)
void SavePlayerState(USSaveGame* SaveObject);

UFUNCTION(BlueprintNativeEvent)
void LoadPlayerState(USSaveGame* SaveObject);



void ASPlayerState::LoadPlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		Credit = SaveObject->Credit;
	}
}
void ASPlayerState::SavePlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		SaveObject->Credit = Credit;
	}
}
```



### 在GameModeBase里保存和读取游戏

`UGameplayStatics `里基本都提供好了方法，直接调用就行

```cpp
UPROPERTY()
USSaveGame* CurrentSaveGame;

FString SlotName;

UFUNCTION(BlueprintCallable, Category = "SaveGame")
void WriteSaveGame();

void LoadSaveGame();



void ASGameModeBase::WriteSaveGame()
{
    for (int32 i = 0; i != GameState->PlayerArray.Num(); ++i)
	{
		ASPlayerState* PlayerState = Cast<ASPlayerState>(GameState->PlayerArray[i]);
		if (PlayerState)
		{
			PlayerState->SavePlayerState(CurrentSaveGame);
			break; // 暂时还没有playerid，所以多人模式的情况下只读取第一个保存的数据
		}
	}
    
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
```



读取游戏要在游戏开始前进行，GameModeBase基类有一个`InitGame`方法可以重写他来实现

```cpp
void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

void ASGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	LoadSaveGame();
}
```



玩家更新数据要发生在创建角色后，而不是上面的游戏开始时，因为游戏开始时可能还没这个玩家

GameModeBase基类有一个`HandleStartingNewPlayer`方法可以重写他来实现

```cpp
void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

void ASGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	ASPlayerState* PlayerState = NewPlayer->GetPlayerState<ASPlayerState>();
	if (PlayerState)
	{
		PlayerState->LoadPlayerState(CurrentSaveGame);
	}
}
```



### 简单用蓝图拖个保存游戏数据验证下

在PlayerController 的蓝图里测试绑定一个键然后执行`WriteSaveGame`

![1697858759266](TyporaPic\1697858759266.png)



## Credit的UI在重新打开游戏的时候还没更新

打断点看了下在我的机器上虽然会先执行GameModeBase的`HandleStartingNewPlayer`，但是在这个函数里执行`LoadPlayerState `之前就已经触发了蓝图的`GetCredit`了，所以数据还没拿到快了一帧，在UI蓝图里加了个延后一帧再执行绑定后调用就可以了





## 根据物品Name保存level里的可交互物品的位置信息

在` SSaveGame` 里加个要保存的数据的结构体数组

```cpp
USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()
        
public:
	UPROPERTY()
	FString ActorName;
    
	UPROPERTY()
	FTransform Transform;
};

UPROPERTY()
	TArray<FActorSaveData> SaveActors;
```



在GameModeBase里找地方写存盘和读取

存的地方可以就直接放在存`PlayerState`的地方，注意Save前先清空Array

读的地方教程是放在` InitGame `里，但是我这里`InitGame`的时候这些可交互物品的Actor还没被创建，最后放在了`StartPlay `和刷怪、刷`PowerUpActor`的计时器放在一起了

```cpp
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
    CurrentSaveGame->SaveActors.Add(ActorSaveData);
}
```



```cpp
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
				break;
			}
		}
	}
}
```



### 保存可交互物品自身的某些数据成员（如宝箱是否打开）

在ItemChest里用 SaveGame 标签可以标记类内需要存盘的数据成员

```cpp
UPROPERTY(ReplicatedUsing = "OnRep_LidOpened", BlueprintReadOnly, SaveGame) // RepNotify
bool bLidOpened;
```



FActorSaveData 加个 `TArray<uint8>` 类型的用来存序列化后的字节

```cpp
USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString ActorName;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> ByteData;
};
```



在`ISGameplayInterface `里加个加载完成的函数，用来处理读取完存盘的数据后的事情

```cpp
UFUNCTION(BlueprintNativeEvent)
void OnActorLoaded();
```



在ItemChest里重写这个函数处理把宝箱盖打开的内容

```cpp
void OnActorLoaded_Implementation() override;

void ASItemChest::OnActorLoaded_Implementation()
{
	OnRep_LidOpened();
}
```



在GameModeBase的存盘和读取的位置处理读写对象的数据成员的内容

用`FMemoryWriter/Reader `写/读数据成员的位置

用`FObjectAndNameAsStringProxyArchive` 处理每个标记了的数据成员的序列化

```cpp
void ASGameModeBase::WriteSaveActorData()
{
    if (!CurrentSaveGame)
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
```



##### 多人游戏情况下的网络复制问题

把ItemChest的`Replicate Movement`  和  `Replicate component `在蓝图里都勾选上





#### 用蓝图做一个交互可以保存的Actor

继承自Actor，加一个Sphere组件和一个`PartialSystem`组件用来展示

![1697973051673](TyporaPic\1697973051673.png)



在`Class Setting`里的Interface 加上`SGameplay Interface`

![1697973061023](TyporaPic\1697973061023.png)



新建一个`bFireActive`的变量，当进行交互了的时候置为true，在高级选项里可以直接勾上保存游戏，和前面在cpp里写的`SaveGame`的标签作用是一样的

![1697973135008](TyporaPic\1697973135008.png)



重写InteractActor的`Interact`事件，每次触发时先修改`bFireActive`的值，然后再保存游戏、更新火焰效果

![1697973209986](TyporaPic\1697973209986.png)



![1697973244097](TyporaPic\1697973244097.png)



![1697973254817](TyporaPic\1697973254817.png)



