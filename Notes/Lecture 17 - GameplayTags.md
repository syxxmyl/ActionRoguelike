# Lecture 17 - GameplayTags

## 添加GameplayTags用于指定某个Action进行时其他Action不能执行

### 在Build.cs里添加`GameplayTags` 的 Module



### 在`ActionComponent`里添加` FGameplayTagContainer  `

用来存储当前有哪些`ActionTags`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
FGameplayTagContainer ActiveGameplayTags;
```

在Tick里打印个`DebugMessage` 方便调试

```cpp
FString DebugStr = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugStr);
```



### 在`SAction `基类里添加 `FGameplayTagContainer`

一个表示自己是哪种Tag，一个表示当哪种Tag在执行的时候自己不能执行

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Tags")
FGameplayTagContainer GrantsTags;

UPROPERTY(EditDefaultsOnly, Category = "Tags")
FGameplayTagContainer BlockedTags;
```



#### 加个函数拿到`ActionComponent`

```cpp
UFUNCTION(BlueprintCallable, Category = "Action")
USActionComponent* GetOwningComponent() const;
	
	
USActionComponent* USAction::GetOwningComponent() const
{
	return Cast<USActionComponent>(GetOuter());
}
```



### 设置标签间的互斥性

如果某个标签的Action正在执行，那么就不允许新的某标签的Action执行

在Action的`CanStart/CanStop`里处理开始执行/结束执行时添加/删除所属的`ActionComponent`的该Action标签

在`ActionComponent` 开始执行/结束执行前用Action的`CanStart/CanStop `来判断能不能执行

```cpp
UFUNCTION(BlueprintNativeEvent, Category = "Action")
bool CanStart(AActor* Instigator);

UFUNCTION(BlueprintNativeEvent, Category = "Action")
bool CanStop(AActor* Instigator);

UFUNCTION(BlueprintCallable, Category = "Action")
bool IsRunning() const;

bool bIsRunning;


bool USAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
		{
			return false;
		}
	}

	return true;
}

bool USAction::CanStop_Implementation(AActor* Instigator)
{
	if (!IsRunning())
	{
		return false;
	}

	return true;
}
```



### 在项目设置里新增`GameplayTags`

新增`Action.Sprinting`和`Action.Attacking`



### 在各个Action蓝图里更新`GrantsTags`和`BlockedTags`

子弹本身的`BlockedTags`也要包含`Action.Attacking`，这样可以禁止同时释放多个类型的Projectile





## 使用上面创建好的Character里的`ActionComponent`做一个类似拉开开关才能打开箱子的功能

在项目设置里增加`KeyCard.Red` `KeyCard.Yellow` `KeyCard.Blue`

改一下之前的`BP_Chest `和 `BP_Layer`

BP_Layer 触发Interact事件的时候获取到Instigator 的 `ActionComponent`，`AddAction` 增加一个颜色类型的Tag

BP_Chest触发Interact事件的时候用`HasTag`先判断一下`Instigator `的`ActionComponent `里有没有某个颜色类型的Tag，如果没有就不打开

### `HasTag`去掉`ExactMatch` 可以变成只要有父标签的Tag也可以返回true

比如你拿了一把红钥匙也能去开蓝钥匙才能开的箱子



## 加一个反弹子弹的Action

### 在项目设置里加个`Status.Parrying`的GameplayTag



### 创建继承自SAction的`ActionParry`

设定好自己是什么`GrantTags`以后在蓝图拖一下当`StartAction`的时候给一个延迟然后执行Stop，这样可以保持这是一个有时限的状态，要把`SAction`的`StopAction`的`UPROPERTY`加个`BlueprintCallable`

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
void StopAction(AActor* Instigator);
```

![1697532053352](TyporaPic\1697532053352.png)



### 在MagicProjectile里设定好怎么反弹

修改`OverLap`事件，当触发重叠的时候判断下`OtherActor`有没有反弹子弹的`ParryTag`的Action正在执行，如果有的话就设置子弹的速度为负当前值，因为在`ProjectileBase`里设置过

```cpp
MovementComp->bRotationFollowsVelocity = true;
```

所以子弹的运动方向会随着速度的变化而变化

为了正确结算伤害等信息，要把这个子弹的`Instigator`设定为`OtherActor`，并且return掉Overlap函数否则就会执行到后面的Explode了

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Damage")
FGameplayTag ParryTag;



void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// 另一种方式拿到GameplayTag
		// static FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag("Status.Parrying");
        
		USActionComponent* ActionComp = USActionComponent::GetActions(OtherActor);
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MovementComp->Velocity *= -1;
			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}

		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
		}

	}
}
```



### 在MagicProjectile的蓝图里设置是拥有哪种能力才能反弹

设置`ParryTag`



### 在Character里设置执行Parry的事件

可以在cpp里写绑定输入，也可以直接在蓝图里拖

![1697531852308](TyporaPic\1697531852308.png)

