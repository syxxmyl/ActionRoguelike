#  Assignment 6 (Rage, Thorns, Power-up, Spotted UI) 

## Rage

类似Health

加个Event事件 `OnRageChanged`触发的时候广播一下，用于修改UI的显示值



## Thorns

要求用cpp做

继承自`ActionEffect`

`StartAction`的时候`AddDynamic `注册 `OnHealthChanged `监听，`StopAction`的时候`RemoveDynamic `注销监听

```cpp
void USActionEffect_Thorns::StartAction_Implementation(AActor* Instigator)
{
	USActionComponent* ActionComp = GetOwningComponent();
	if (ActionComp)
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(ActionComp->GetOwner());
		if (AttributeComp)
		{
			AttributeComp->OnHealthChanged.AddDynamic(this, &USActionEffect_Thorns::OnHealthChanged);
		}
	}
}

void USActionEffect_Thorns::StopAction_Implementation(AActor* Instigator)
{
	USActionComponent* ActionComp = GetOwningComponent();
	if (ActionComp)
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(ActionComp->GetOwner());
		if (AttributeComp)
		{
			AttributeComp->OnHealthChanged.RemoveDynamic(this, &USActionEffect_Thorns::OnHealthChanged);
		}
	}
}

void USActionEffect_Thorns::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta >= 0.0f)
	{
		return;
	}

	USActionComponent* ActionComp = GetOwningComponent();
	if (!ActionComp)
	{
		return;
	}
	AActor* OwningActor = ActionComp->GetOwner();
	if (!OwningActor || OwningActor == InstigatorActor)
	{
		return;
	}

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorActor);
	if (AttributeComp)
	{
		int32 ThornDamage = FMath::RoundToInt(Delta * ThornsDamageWeight);
		if (ThornDamage == 0)
		{
			return;
		}

		AttributeComp->ApplyHealthChange(OwningActor, ThornDamage);
		UE_LOG(LogTemp, Log, TEXT("%s make %d ThornDamage by %f Original Damage"), *GetNameSafe(ActionComp->GetOwner()), ThornDamage, Delta);
	}
}
```



## 给AI加一个看到玩家头顶冒个！的umg

继承自`USWorldUserWidget`的蓝图，在`AICharacter`里写创建UMG的逻辑

需要当看到的Pawn不是当前的`TargetActor`才行，找个变量存一下每次Set的Actor，创建前比较一下

```cpp
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<UUserWidget> PlayerSpottedWidgetClass;

AActor* CurrentTargetActor;




void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
    if (CurrentTargetActor != Pawn)
    {
		USWorldUserWidget* PlayerSpottedWidget = CreateWidget<USWorldUserWidget>(GetWorld(), PlayerSpottedWidgetClass);
		if (PlayerSpottedWidget)
		{
			PlayerSpottedWidget->BindAttachedActor(this);
			PlayerSpottedWidget->AddToViewport();
		}
    }

    SetTargetActor(Pawn);
    DrawDebugString(GetWorld(), GetActorLocation(), "SET TARGET PLAYER", nullptr, FColor::White, 4.0f, true);
}

void ASAICharacter::SetTargetActor(AActor* TargetActor)
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		AIController->GetBlackboardComponent()->SetValueAsObject("TargetActor", TargetActor);
	}

    CurrentTargetActor = TargetActor;
}
```

### 简单做个UMG

因为继承自`USWorldUserWidget`，所以需要一个`SizeBox`，然后在`SizeBox`里加个Text文本框文本内容是`！`

做个简单的2秒动画，文本框放大再缩小回原比例

当UMG构造的时候播放动画，延迟和动画相同的秒数后销毁





## 创建一个PowerUpAction用来激活各种Action

可以激活哪种Action设置为`EditAnywhere`，这样方便用不同的实例激活不同的Action

每次加之前判断下`ActionComp`是否已经有这种Action了，有的话就不加

```cpp
UPROPERTY(EditAnywhere, Category = "Action")
TSubclassOf<USAction> ActivateActionClass;


bool USActionComponent::HasAction(TSubclassOf<USAction> ActionClass)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->IsA(ActionClass))
		{
			return true;
		}
	}

	return false;
}


void ASPowerUpActor_ActionActivate::Interact_Implementation(APawn* InstigatorPawn)
{
	USActionComponent* ActionComp = USActionComponent::GetActions(InstigatorPawn);
	if (!ActionComp)
	{
		return;
	}

	if (ActionComp->HasAction(ActivateActionClass))
	{
		return;
	}

	ActionComp->AddAction(InstigatorPawn, ActivateActionClass);	
}
```



## Assignment6 修改

### Rage和Health变化时的事件参数是一样的，可以不用分开设置监听事件类型而是用同一个就行

```cpp
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewHealth, float, Delta);

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRageChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewRage, float, Delta);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewRage, float, Delta);


UPROPERTY(BlueprintAssignable)
FOnAttributeChanged OnHealthChanged;

UPROPERTY(BlueprintAssignable)
FOnAttributeChanged OnRageChanged;
```



### 当玩家被发现时加的Viewport的权重值提高一些

这样可以避免当生命值血条和发现时的感叹号同时出现时的错位问题

之前做的时候是设置了不同的高度

值越大越靠前

```cpp
PlayerSpottedWidget->AddToViewport(10);
```


