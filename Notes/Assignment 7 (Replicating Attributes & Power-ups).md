#  Assignment 7 (Replicating Attributes & Power-ups) 

## Credit 改成Replicated的

给Credit加个`ReplicatedUsing `描述

```cpp
UPROPERTY(VisibleAnyWhere, ReplicatedUsing = "OnRep_CreditChanged", Category = "PlayerState")
float Credit;
```

### OnRep_XXX 可以带一个参数，类型和变化的类型相同，客户端收到的时候这个参数就是变化前的值

这样就不用Multicast传`Delta`过来了，可以用这个特性算出来Delta是多少

```cpp
UFUNCTION()
void OnRep_CreditChanged(float OldCredit);

void ASPlayerState::OnRep_CreditChanged(float OldCredit)
{
	UE_LOG(LogTemp, Log, TEXT("Client OnRep_CreditChanged: Credit is %f, Delta is %f"), Credit, Credit - OldCredit);
	OnCreditChanged.Broadcast(this, Credit, Credit - OldCredit);
}
```



### 只有Server才能修改Credit的值

目前来看一共有两种情况修改Credit的值，一种是和InteractActor互动，另一种是击杀AI怪物，这两种情况都只会在服务器的代码里处理，所以已经不需要修改了



## 修一下积分UMG客户端会报错GetPlayerState为空的问题

因为客户端和服务器通信需要时间，而客户端的`PlayerState`是服务端下发下来的，所以现在的情况是客户端的`PlayerController`的`BeginPlay`执行的时候创建`Main_HUD`，`Main_HUD`创建`Credit_Widget`的时候执行`Credit_Widget`的Construct绑定需要用到`PlayerState`，但是这个时候可能服务器还没有把`PlayerState`下发下来，所以会绑定失败报错

### 简单粗暴的解决方法

创建`Main_HUD`的时间往后延一点，这样服务器就下发下来了

治标不治本，要从根源解决问题

### `PlayerController` 有提供复制`PlayerState`完成时触发的事件

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, APlayerState*, NewPlayerState);

virtual void BeginPlayingState() override;

UFUNCTION(BlueprintImplementableEvent)
void BlueprintBeginPlayingState();

void OnRep_PlayerState() override;

UPROPERTY(BlueprintAssignable)
FOnPlayerStateChanged OnPlayerStateReceived;




void ASPlayerController::BeginPlayingState()
{
	BlueprintBeginPlayingState();
}

void ASPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReceived.Broadcast(PlayerState);
}
```



## PowerUpActor 客户端可以正确显示/隐藏

PowerUp只会在服务端调用Interact的时候调用，所以不用改成服务端远程调用

把是否显示设置为Replicated，每次值变化的时候修改碰撞和可见性即可

```cpp
UPROPERTY(ReplicatedUsing = "OnRep_VisibleChanged", BlueprintReadOnly)
bool bIsVisible;

UFUNCTION()
void OnRep_VisibleChanged();




void ASPowerUpActor::ResetCD()
{
	bIsVisible = true;
	OnRep_VisibleChanged();
}

void ASPowerUpActor::OnRep_VisibleChanged()
{
	SetActorEnableCollision(bIsVisible);
	RootComponent->SetVisibility(bIsVisible, true);	
}

bool ASPowerUpActor::PowerUp()	
{
	if (bIsVisible)
	{
		bIsVisible = false;
		OnRep_VisibleChanged();
		GetWorldTimerManager().SetTimer(TimerHandle_CD, this, &ASPowerUpActor::ResetCD, PowerUpCDTime);
		return true;
	}

	return false;
}
```



## Rage改成和Health一样的Replicated

照着Health复制

这俩现在都不是只有Server端能调用，后面要改



## 客户端也能正确显示AI发现玩家时头上的感叹号

把create widget 改成广播事件里执行





## Assignment7 修改

### Credit网络同步问题

上一章读取存储的`PlayerState`的Credit的时候直接赋值了，应该用`ApplyCreditChange` 这样能触发`OnCreditChanged` 事件的广播

```cpp
void ASPlayerState::LoadPlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
        // Credit = SaveObject->Credit;
		ApplyCreditChange(SaveObject->Credit);
	}
}
```



### 上一章说的Credit的UI打开游戏的时候不会立刻更新之前保存的数值

上一章是用delay延后了一帧再创建UI，但是治标不治本，根本原因是

`HandleStartingNewPlayer_Implementation` 之前是先调用`Super`的函数再`LoadPlayerState`，但是Super的时候就会执行` PlayerController`的 `BeginPlayingState` ，调用` BlueprintBeginPlayingState` 创建`Main_HUD`，所以改成先读取保存数据再调用父类的函数

```cpp
void ASGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ASPlayerState* PlayerState = NewPlayer->GetPlayerState<ASPlayerState>();
	if (PlayerState)
	{
		PlayerState->LoadPlayerState(CurrentSaveGame);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}
```



