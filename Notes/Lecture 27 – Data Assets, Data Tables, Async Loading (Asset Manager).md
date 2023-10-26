# Lecture 27 – Data Assets, Data Tables, Async Loading (Asset Manager)

## DataTable

https://docs.unrealengine.com/5.1/zh-CN/data-driven-gameplay-elements-in-unreal-engine/



## 在cpp里实现要读取的DataTable的结构

在GameModeBase里，实现一张生成Monster的数据表



```cpp
USTRUCT(BlueprintType)
struct FMonsterInfoRow : public FTableRowBase
{
	GENERATED_BODY()
        
public:
	FMonsterInfoRow()
	{
		Weight = 1.0f;
		SpawnCost = 5.0f;
		KillReward = 20.0f;
	}
    
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> MonsterClass;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnCost;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float KillReward;
};
```



## 在Content里新建一张结构为`FMonsterInfoRow `的DataTable `DT_Monsters`

目前先指定两种怪，普通怪权重更高但是奖励更少，精英怪权重较低但是奖励更高

![1698141948689](TyporaPic\1698141948689.png)



## 根据数据表的内容生成Monster

```cpp
void ASGameModeBase::OnSpawnBotQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	// ...
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

			GetWorld()->SpawnActor<ASAICharacter>(SelectedRow->MonsterClass, Locations[0], FRotator::ZeroRotator, SpawnParams);
		}
		// ...
	}
}
```



## DataAsset

纯粹的数据保存



## 用DataAsset完善上面的DataTable

创建一个继承自`UPrimaryDataAsset`的类` USMonsterData`

指定了怪物的类，怪物有哪些动作，怪物的图标

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
TSubclassOf<AActor> MonsterClass;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
TArray<TSubclassOf<USAction>> Actions;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
UTexture2D* Icon;
```



修改DataTable的数据结构

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly)
USMonsterData* MonsterData;
	
	
	
void ASGameModeBase::OnSpawnBotQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	// ...
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
	// ...
}
```



### 在Content里创建DataAsset

创建一个DataAsset，然后填写里面的内容

回到DataTable里，把DataAsset填到表里



## Hard & Soft References

https://docs.unrealengine.com/5.3/zh-CN/referencing-assets-in-unreal-engine/

### SizeMap

在Content里可以右击选择`Size Map`打开，可以看到加载这个资源的时候有哪些内容要加载

### Reference Viewer

在Content里可以右击选择`Reference Viewer`打开，可以看到引用了哪些类以及被哪些类引用



### 制作一个软引用Soft Reference

以Action的显示的图标Icon为例

用`TSoftObjectPtr `包裹它

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
TSoftObjectPtr<UTexture2D> Icon;
```



#### 蓝图中加载软引用

原本直接使用的地方现在会报错，需要先加载资源

![1698146089234](TyporaPic\1698146089234.png)



`Async Load Asset` ->` Completed `->` Cast to Texture2D`(转换为纯类型转换) -> `set value`

![1698146372192](TyporaPic\1698146372192.png)



## 用Asset Manager管理异步资源加载 Async Loading

https://docs.unrealengine.com/5.3/zh-CN/asynchronous-asset-loading-in-unreal-engine/



### 把MonsterTable 里的MonsterData 改成 FPrimaryAssetId

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly)
FPrimaryAssetId MonsterId;
```



### SMonsterData 重写一个GetPrimaryAssetId

```cpp
FPrimaryAssetId GetPrimaryAssetId() const override;


FPrimaryAssetId USMonsterData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("Monsters", GetFName());
}
```



### 在项目设置里的AssetManager里新增AssetType

新增一个Monsters的类型，基础类是`SMonsterData`，目录就选择`Content`里的`Monsters`文件夹

![1698147267145](TyporaPic\1698147267145.png)



### 修改DT_Monsters 数据表里的MonsterId

![1698147705823](TyporaPic\1698147705823.png)





### 在cpp里处理上面修改的资源的异步加载

原本spawn Monster的地方改成先异步加载资源，加载完成了再`Spawn Monster`

用`UAssetManager `来管理资源的加载

申请加载资源的时候传入一个绑定了` OnMonsterLoaded `的`FStreamableDelegate`类型的委托，可以动态传入多个参数

加载成功后执行绑定委托的回调，从`UAssetManager `根据`LoadedId`拿到加载的资源`USMonsterData`，然后执行`Spawn Monster`的操作

```cpp
void OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation);



void ASGameModeBase::OnSpawnBotQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	// ...
    LogOnScreen(this, "Loading monster...", FColor::Green);

    TArray<FName> Bundles;
    FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ASGameModeBase::OnMonsterLoaded, SelectedRow->MonsterId, Locations[0]);
    
    Manager->LoadPrimaryAsset(SelectedRow->MonsterId, Bundles, Delegate);
	// ...
}

void ASGameModeBase::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	UAssetManager* Manager = UAssetManager::GetIfValid();
	if (!Manager)
	{
		return;
	}

	USMonsterData* MonsterData = Cast<USMonsterData>(Manager->GetPrimaryAssetObject(LoadedId));
	if (!MonsterData)
	{
		return;
	}
	
	LogOnScreen(this, "Finished loading.", FColor::Green);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewBot = GetWorld()->SpawnActor<ASAICharacter>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewBot)
	{
		LogOnScreen(this, FString::Printf(TEXT("Spawned enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(MonsterData)));
		USActionComponent* ActionComp = USActionComponent::GetActions(NewBot);
		if (ActionComp)
		{
			for (TSubclassOf<USAction> ActionClass : MonsterData->Actions)
			{
				ActionComp->AddAction(NewBot, ActionClass);
			}
		}
	}

	DrawDebugSphere(GetWorld(), SpawnLocation, 50.0f, 20, FColor::Blue, false, 60.0f);
}
```

