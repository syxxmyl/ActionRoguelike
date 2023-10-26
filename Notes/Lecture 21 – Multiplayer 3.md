# Lecture 21 – Multiplayer 3

## 修改Action为网络同步复制

https://docs.unrealengine.com/5.1/zh-CN/replicated-subobjects-in-unreal-engine/



### 加点日志

#### 包装一层打印日志的函数用来区分是哪端打印的

在项目名的头文件里

```cpp
static void LogOnScreen(UObject* WorldContext, FString Msg, FColor Color = FColor::White, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}
```

#### ActionComponent的每帧打印的日志改一下

之前是只打印正在执行的`ActionTags`，现在改成所有注册了的Action都打印

```cpp
// FString DebugStr = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugStr);

for (USAction* Action : Actions)
	{
		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;

		FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s : IsRunning: %s : Outer: %s"),
			*GetNameSafe(GetOwner()),
			*Action->ActionName.ToString(),
			Action->IsRunning() ? TEXT("true") : TEXT("false"),
			*GetNameSafe(Action->GetOuter()));

		LogOnScreen(this, ActionMsg, TextColor, 0.0f);
	}
```



### 复制`ActionComponent`里存放的注册了的Action 子对象

标记 Actions 是为了在客户端那边能在`ActionComponent`里找到Action，因为复制Action只会复制它本身，不会复制它的引用信息

复制子对象要用` ReplicateSubobjects `把子对象注册到复制Channel里，重写一下

 当该Channel所属Actor被同步到客户端时，注册了的Action 也会作为Actor的`Subobject`同步到了客户端 

```cpp
UPROPERTY(Replicated)
TArray<USAction*> Actions;

virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;



bool USActionComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (USAction* Action : Actions)
	{
		if (Action)
		{
			WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}

void USActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USActionComponent, Actions);
}
```



### 只允许服务端注册默认Action列表

`BeginPlay`的时候判断下有没有权限，有权限才能注册

```cpp
void USActionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<USAction> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}
}
```



### 复制 继承自 UObject 的Actor对象

因为UObject没有提供类似`SetReplicate`之类的简单的方法，需要重写` IsSupportedForNetworking `方法， 用来标记`UObject`是否支持复制 ，返回true 代表支持网络复制

开始/结束Action 通过复制`bIsRunning`来做，暂时还没处理要传入的Instigator的内容，后面再处理，现在先传`nullptr`，现在只会影响非触发者的其他客户端，因为申请执行Action的客户端知道`Instigator`，服务端收到了客户端传来的`Instigator`，只有其他通过`bIsRunning`变化执行`OnRef`的客户端不知道`Instigator`

记得要在 `GetLifetimeReplicatedProps  `里注册复制的对象

```cpp
bool IsSupportedForNetworking() const override;

UFUNCTION()
void OnRep_IsRunning();
	
UPROPERTY(ReplicatedUsing = "OnRep_IsRunning")
bool bIsRunning;



void USAction::OnRep_IsRunning()
{
	if (bIsRunning)
	{
		StartAction(nullptr);
	}
	else
	{
		StopAction(nullptr);
	}
}

bool USAction::IsSupportedForNetworking() const
{
	return true;
}

void USAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USAction, bIsRunning);
}
```



### 修改Action的Outer为`ActionComponent`的Owner

之前的Outer是`ActionComponent`本身，但是在网络复制的时候复制到客户端会改成`ActionComponent`的Owner的Actor

在Action身上加一个变量指向拥有它的`ActionComponent`，然后在` ActionComponent` 的 `AddAction` 创建成功后进行赋值

```cpp
UPROPERTY(Replicated)
USActionComponent* ActionComp;

void Initialize(USActionComponent* NewActionComp);



void USAction::Initialize(USActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}

void USAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // ...
	DOREPLIFETIME(USAction, ActionComp);
}

void USActionComponent::AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass)
{
	// ...

	USAction* NewAction = NewObject<USAction>(GetOwner(), ActionClass);
	if (ensure(NewAction))
	{
		NewAction->Initialize(this);
		// ...
	}
}
```



原本的`GetOwningComponent`也可以改成用这个变量了

```cpp
USActionComponent* USAction::GetOwningComponent() const
{
	return ActionComp;
}
```



#### RepNotify 只有当收到服务器传来的值和此刻客户端的值不同的时候才会执行，如果客户端自己也跑了逻辑修改了值变得和服务器一样就不会执行了