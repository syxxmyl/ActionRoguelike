# Lecture 18 – Creating _Buffs_, World Interaction

## 基于前面的Action创建一个BUFF系统

### USActionEffect

创建一个新类 USActionEffect 继承自 `USAction`

`Duration` 是这个Buff的总持续时间

`Period `是这个Buff的触发间隔时间

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
float Duration;
	
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
float Period;
```

当这个`USAction`执行`StartAction`的时候设置两个定时器

基于Duration的到时间触发`StopAction`，只触发一次

基于Period的到时间触发具体的逻辑，会一直触发

```cpp
UFUNCTION(BlueprintNativeEvent, Category = "Effect")
void ExecutePeriodicEffect(AActor* Instigator);



void USActionEffect::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	if (Duration > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "StopAction", Instigator);

		GetWorld()->GetTimerManager().SetTimer(DurationHandle, Delegate, Duration, false);
	}

	if (Period > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "ExecutePeriodicEffect", Instigator);

		GetWorld()->GetTimerManager().SetTimer(PeriodHandle, Delegate, Period, true);
	}
}
```

在`StopAction`里执行父类的`StopAction`前要判断一下

最后一次的Period剩余间隔如果是低于`KINDA_SMALL_NUMBER` 的，要执行最后一次`ExecutePeriodicEffect`，因为最后一次的Period触发可能会在`DUration`触发之后，导致已经走到下面的`ClearTimer`了，不能因为这个把最后一次执行给吞了

```cpp
void USActionEffect::StopAction_Implementation(AActor* Instigator)
{
	if (GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < KINDA_SMALL_NUMBER)
	{
		ExecutePeriodicEffect(Instigator);
	}

	Super::StopAction_Implementation(Instigator);

	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);
	GetWorld()->GetTimerManager().ClearTimer(PeriodHandle);

	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->RemoveAction(this);
	}
}
```



### USAction要增加的部分

加一个是否自动执行的变量

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Action")
bool bAutoStart;
```



#### USActionEffect的构造函数设自动执行为true

```cpp
USActionEffect::USActionEffect()
{
	bAutoStart = true;
}
```



### USActionComponent要增加的部分

在`AddAction` 的时候判断一下这个Action是否设置了自动开始，如果设置了就在`AddAction`成功后直接执行对应的`StartAction`

```cpp
void USActionComponent::AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	USAction* NewAction = NewObject<USAction>(this, ActionClass);
	if (ensure(NewAction))
	{
		Actions.Add(NewAction);

		if (NewAction->bAutoStart && ensure(NewAction->CanStart(Instigator)))
		{
			NewAction->StartAction(Instigator);
		}
	}
}
```

#### 加一个`RemoveAction`，取消某个Action的注册

```cpp
UFUNCTION(BlueprintCallable, Category = "Actions")
void RemoveAction(USAction* ActionToRemove);


void USActionComponent::RemoveAction(USAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	Actions.Remove(ActionToRemove);
}
```

### 在项目设置里加一个新的Status.Burning



## 创建一个新的基于USActionEffect的蓝图类BP_EffectBurning

配置好伤害量、持续时间、触发间隔、标签

在蓝图里重写`ExecutePeriodicEffect `函数，用之前创建的 `USGameplayFunctionLibrary` 蓝图库给于目标一个持续燃烧掉血的效果

![1697539568706](TyporaPic\1697539568706.png)



### MagicProjectile 加个Overlap的时候执行`BurningEffect`

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Burning")
TSubclassOf<USActionEffect> EffectClass;

void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// ...

		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
			if (ActionComp)
			{
				ActionComp->AddAction(GetInstigator(), EffectClass);
			}
		}

	}
}
```



## 当摄像机聚焦于一个可互动物体时展示一个Widget

### 修改`InteractionComponent`

原本是触发Interact的时候才去做射线检测然后拿到命中的`InteractActor`，现在改成每帧都检测，如果检测到了就在这个Actor上展示一个Widget，如果没检测到就不展示

每帧检测到的Actor暂时存一下，用来在Interact的时候能知道要和哪个Actor互动	标识为`UPROPERTY`的目的是为了让虚幻的垃圾回收系统帮忙处理聚焦的Actor析构了自动把这里的指针也置为`nullptr`这件事

创建好的Widget实例也存一下，这样只有第一次Tick的时候会创建，之后都可以直接复用这个了



```cpp
UPROPERTY()
AActor* FocusedActor;

UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<USWorldUserWidget> DefaultWidgetClass;

UPROPERTY()
USWorldUserWidget* DefaultWidgetInstance;




FocusedActor = nullptr;
for (auto& Hit : OutHits)
{
   //...
    
    AActor* HitActor = Hit.GetActor();
    if (HitActor)
    {
        if (HitActor->Implements<USGameplayInterface>())
        {
            FocusedActor = HitActor;
            break;
        }
    }
}

if (FocusedActor)
{
    if (!DefaultWidgetInstance && ensure(DefaultWidgetClass))
    {
        DefaultWidgetInstance = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
    }

    if (DefaultWidgetInstance)
    {
        DefaultWidgetInstance->BindAttachedActor(FocusedActor);
        if (!DefaultWidgetInstance->IsInViewport())
        {
            DefaultWidgetInstance->AddToViewport();
        }
    }
}
else
{
    if (DefaultWidgetInstance)
    {
        DefaultWidgetInstance->RemoveFromParent();
    }
}
```



### 加个新的Widget

记得加个`SizeBox`用在`USWorldUserWidget`里

修改Character的`InteractionComp`里的`DefaultWidgetClass`