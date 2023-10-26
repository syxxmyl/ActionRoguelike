# Lecture 16 - Writing our own _Gameplay Ability System_ alternative

## 创建一个Action系统

### 创建`ActionComponent `处理动作

主要包括`注册Action`，`执行Action`和`停止Action`，依靠类型为`FName`的`ActionName`来判断要对哪个Action做操作

`AddAction`的时候用`NewObject`指定了Action的`Outer`对象，用于后面`GetWorld`拿到`UWorld`的访问权限

```cpp
UFUNCTION(BlueprintCallable, Category = "Actions")
void AddAction(TSubclassOf<USAction> ActionClass);

UFUNCTION(BlueprintCallable, Category = "Actions")
bool StartActionByName(AActor* Instigator, FName ActionName);

UFUNCTION(BlueprintCallable, Category = "Actions")
bool StopActionByName(AActor* Instigator, FName ActionName);

UPROPERTY()
TArray<USAction*> Actions;

void USActionComponent::AddAction(TSubclassOf<USAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}
	USAction* NewAction = NewObject<USAction>(this, ActionClass);
	if (ensure(NewAction))
	{
		Actions.Add(NewAction);
	}
}

bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			Action->StartAction(Instigator);  
			return true;
		}
	}
	return false;
}

bool USActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			Action->StopAction(Instigator);
			return true;
		}
	}
	return false;
}
```



### 创建`SAction`基类供后续各种动作继承

继承自`UObject`

加上` UCLASS(Blueprintable) `  来指定可以被蓝图类继承

用`BlueprintNativeEvent` 修饰执行函数，可以在蓝图重写逻辑

```cpp
UFUNCTION(BlueprintNativeEvent, Category = "Action")
void StartAction(AActor* Instigator);

UFUNCTION(BlueprintNativeEvent, Category = "Action")
void StopAction(AActor* Instigator);

UPROPERTY(EditDefaultsOnly, Category = "Action")
FName ActionName;
```



### 把Character里的`SpawnProjectile`改成借助`ActionComponent`执行`Action_Projectile`动作



#### `SpawnActor`等函数需要`GetWorld`，这个`GetWorld`需要`SAction`重写继承自`UObject`的函数才能正确提供

`GetOuter` 拿到的就是在`ActionComponent`里`AddAction`的时候`NewObject`指定的Outer对象

```cpp
UWorld* GetWorld() const override;

UWorld* USAction::GetWorld() const
{
	UActorComponent* Comp = Cast<UActorComponent>(GetOuter());
	if (Comp)
	{
		return Comp->GetWorld();
	}
	return nullptr;
}
```



#### 创建完子弹Actor后要手动执行`StopAction`



### 在`ActionComponent`里暴露给蓝图一个默认初始化Action类的TArray，用来处理Action在哪`AddAction`的问题

在`ActionComponent`的蓝图里设置好要执行哪些Action的默认初始化操作，然后在`BeginPlay`的时候执行`AddAction`把这些Action绑定到Component里



```cpp
UPROPERTY(EditAnyWhere, Category = "Actions")
TArray<TSubclassOf<USAction>> DefaultActions;


void USActionComponent::BeginPlay()
{
	Super::BeginPlay();
	for (TSubclassOf<USAction> ActionClass : DefaultActions)
	{
		AddAction(ActionClass);
	}
	
}
```



