# Lecture 22 – Finishing up Multiplayer

## 处理 MagicProjectile 在服务端和客户端表现不一致的问题

### 处理ProjectileAction的Instigator在OnRep里之前一直填的是nullptr的问题

把`bIsRunning` 和 `Instigator `封装到一个结构体里，原本是注册`bIsRunning `是否变化，现在改成注册这个结构体是否有变化



```cpp
USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	AActor* InstigatorActor;
    
	UPROPERTY()
	bool bIsRunning;
};

UFUNCTION()
void OnRep_ActionRepDataChanged();

UPROPERTY(ReplicatedUsing = "OnRep_ActionRepDataChanged")
FActionRepData ActionRepData;



void USAction::StartAction_Implementation(AActor* Instigator)
{
	// ...
	ActionRepData.bIsRunning = true;
	ActionRepData.InstigatorActor = Instigator;
}

void USAction::StopAction_Implementation(AActor* Instigator)
{
	// ...
	ActionRepData.bIsRunning = false;
	ActionRepData.InstigatorActor = Instigator;
}

void USAction::OnRep_ActionRepDataChanged()
{
	if (ActionRepData.bIsRunning)
	{
		StartAction(ActionRepData.InstigatorActor);
	}
	else
	{
		StopAction(ActionRepData.InstigatorActor);
	}
}

void USAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// ...
	DOREPLIFETIME(USAction, ActionRepData);
	// ...
}
```



## 处理AttributeComp里Health能在客户端变更的问题

把`Health变更`、`广播Health变更事件`、`击杀通知GameModeBase处理积分`这些都包到`HasAuthority`里，只有服务端才处理这些，客户端值只返回受到的伤害是否为0

```cpp
bool USAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// ...

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, HealthMax);
	float RealDelta = NewHealth - OldHealth;

	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;
		if (RealDelta != 0.0f)
		{
			MulticastHealthChanged(InstigatorActor, Health, RealDelta);
		}

		if (RealDelta < 0.0f && Health == 0.0f)
		{
			ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
			if (GM)
			{
				GM->OnActorKilled(GetOwner(), InstigatorActor);
			}
		}
	}

	return RealDelta != 0;
}
```



## 处理客户端和服务端都执行了一次StartAction导致生成了两个Projectile的问题

把定时器创建Projectile的也给改成只有服务端能执行

```cpp
if (Character->HasAuthority())
{
    FTimerHandle TimerHandle_AttackDelay;
    FTimerDelegate Delegate;
    Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, ProjectileSpawnDelayTime, false);
}
```



## Projectile触发Overlap的时候禁止客户端直接执行AddAction

用`HasAuthority `校验下，`AddAction里`以防万一也加上`HasAuthority`

```cpp
void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// ...

		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
			if (ActionComp && HasAuthority())
			{
				ActionComp->AddAction(GetInstigator(), EffectClass);
			}
		}

	}
}

void USActionComponent::AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client attempting to AddAction. [Class: %s]"), *GetNameSafe(ActionClass));
		return;
	}

	// ...
}
```



## Sprint客户端没有主动StopAction

之前只改了`ActionComponent`的` StartAction`变成了Server的RPC，照着把`StopAction`也改了

```cpp
UFUNCTION(Server, Reliable)
void ServerStopActionByName(AActor* Instigator, FName ActionName);




void USActionComponent::ServerStopActionByName_Implementation(AActor* Instigator, FName ActionName)
{
	StopActionByName(Instigator, ActionName);
}

bool USActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			// ...

			if (!GetOwner()->HasAuthority())
			{
				ServerStopActionByName(Instigator, ActionName);
			}

			Action->StopAction(Instigator);
			return true;
		}
	}

	return false;
}
```



## 处理玩家死亡复活后Health的UMG错误的问题

本质原因是UMG绑定的还是死了的那个Pawn的相关数据，玩家复活后是一个新的Pawn了

这个事件会在`PlayerController`里调用`SetPawn`函数，所以重写下这个函数，广播一个新的事件`FOnPawnChanged`，并且暴露给蓝图调用即可

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnChanged, APawn*, NewPawn);

UPROPERTY(BlueprintAssignable)
FOnPawnChanged OnPawnChanged;

virtual void SetPawn(APawn* InPawn) override;




void ASPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	OnPawnChanged.Broadcast(InPawn);
}
```



原本UMG的事件构造是直接绑定了`OnHealthChanged `事件的，现在在之前加一个事件绑定

事件构造先 `Get Owning Player` -> ` Cast to SplayerController` -> `Assign on Pawn Changed`先绑定上面刚创建的PlayerController绑定Pawn的事件，然后在这个事件触发的时候再去绑定`OnHealthChanged`

![1697803298174](TyporaPic\1697803298174.png)



## 人物死亡后过几秒消失，不要一直留在level里

`OnHealthChanged`里判断当前Health小于0时用` SetLifeSpan` 设置一下

```cpp
void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	// ...
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		// ...
		SetLifeSpan(5.0f);
	}
}
```

