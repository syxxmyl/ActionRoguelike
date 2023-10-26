#  Assignment 5 (Credits System, EQS Spawning) 

## PlayerState

用继承自`PlayerState`的类存放Credit，然后在`GameMode`里设置`PlayerState`为这个即可



## CreditAmountChange

当成功变化数值的时候广播一个event，留给UI显示用

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditChanged, ASPlayerState*, OwningPlayerState, float, NewCredit, float, Delta);
```



## 使用治疗药水消耗积分，新增一个交互获取积分的`PowerUpActor_Coin`类



## UI监听`OnCreditChanged`事件实时变化Credit数量



## 游戏开始时执行EQS查询在地图上随机生成治疗药水和硬币

大体和生成`MinionRnaged `差不多



```cpp
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
```

