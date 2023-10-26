# Lecture 12 - More AI, Environment Query Spawn Logic

## 通过EQS查询寻找Bot自动生成点位

### 不启动游戏快捷查看EQS查询结果

可以新建一个蓝图继承自 `EQSTestingPawn` ，把他放到关卡编辑器中在细节栏里设置EQS模板就能看到查询的结果了，并且会随着EQS的修改而实时更新查询结果



### 有些EQS的点位在体积巨大的Cube内部的NavMesh点位，导致如果生成在这个点位会卡在里面

##### 常规做法

用`NavModifierVolume` 来把每个类似情况的Cube内部的`NavMesh`给清除掉

然后在EQS的查询细节里有 `Projection Data`， `TraceMode` 选上`NavMesh `，这样生成在内部的点位就会挪到最近的`NavMesh`上了

##### 适合原型设计快速修改的做法

在EQS加一个寻路查询`PathFinding`的Test，`Context`是`GamePlayer`，这样只会生成在能寻路到达GamePlayer的点位



## 创建一个新的`GameModeBase`

### 在`GameModeBase`里定时生成`MinionRanged Bot`

`GameModeBase `是用来通知各Actor执行`BeginPlay`的，所以这个类的开始是另一个` StartPlay`

在`StartPlay`里设置一个定时器每次触发的时候执行一次EQS查询

```cpp
GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnTimerInterval, true);
```



EQS查询可能需要好几帧才能完成，所以查询的结果需要动态绑定响应事件

```cpp
UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);

if (ensure(QueryInstance))
{
    QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnQueryCompleted);
}
```



首先先判断查询是否成功， 然后选择查询结果数组的第一个下标存放的Location作为bot Spawn 的位置

```cpp
if (QueryStatus != EEnvQueryStatus::Success)
{
    UE_LOG(LogTemp, Warning, TEXT("Spawn Bot EQS Query Failed!"));
    return;
}

TArray<FVector>Locations = QueryInstance->GetResultsAsLocations();

if (Locations.Num() > 0)
{
    GetWorld()->SpawnActor<AActor>(MinionClass, Locations[0], FRotator::ZeroRotator);
}
```



### 使`GameModeBase`生效

在关卡编辑器创建一个该class的BP，然后设置需要的assets，然后在窗口->世界场景设置里把游戏模式换成这个蓝图，这样在这个level里就是使用这个`GameMode`了



### 没生成`MinionRanged`

打断点调试看`SpawnActor`返回的是 `nullptr` ，于是仿照前面生成子弹加了个`SpawnParam`

试了下`AdjustIfPossibleButAlwaysSpawn `和`  AlwaysSpawn `可以正常生成

https://zhuanlan.zhihu.com/p/411549290

应该是和碰撞设置有关，暂时先通过设定参数的形式忽略

后续：

加了个`DrawDebugSphere` 发现Location有一半在地下（

```cpp
if (Locations.Num() > 0)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    GetWorld()->SpawnActor<ASAICharacter>(MinionClass, Locations[0], FRotator::ZeroRotator, SpawnParams);
}
```



### 生成的`MinionRanged` 一动不动

因为还没有被`AIController`接管，在`SAICharacter`的构造函数加上

```cpp
AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
```



### 用`CurveFloat` 来规定Bot生成的数量

设定几个结点，在时间到达前bot的数量不能超过设定值

bot的数量可以用`ActorIterator` 遍历拿到

```cpp
int32 NearOFAliveBots = 0;
for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
{
    ASAICharacter* Bot = *It;
    USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(Bot->GetComponentByClass(USAttributeComponent::StaticClass()));
    if (AttributeComp && AttributeComp->GetHealth() > 0.0f)
    {
        ++NearOFAliveBots;
    }
}

float MaxBotCount = 10.0f;
if (DifficultyCurve)
{
    MaxBotCount = DifficultyCurve->GetFloatValue(GetWorld()->GetTimeSeconds());
}

if (NearOFAliveBots >= MaxBotCount)
{
    return;
}
```

