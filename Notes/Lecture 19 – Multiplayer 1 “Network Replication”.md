# Lecture 19 – Multiplayer 1 “Network Replication”

https://docs.unrealengine.com/5.3/zh-CN/networking-and-multiplayer-in-unreal-engine/



## 把和宝箱交互改成服务器RPC

### 创建RPC函数

在`InteractionComponent `里加一个服务器RPC的函数，用`UFUNCTION`修饰它

`Server` 标签表示这个函数运行在服务器端

`Reliable` 标签表示为这个函数提供可靠交付，确保服务器能收到客户端的RPC请求并运行它

```cpp
// Reliable - Will always arrive, eventually. Request will be re-sent unless an acknowledgment was received.
// Unreliable - Not guaranteed, packet can get lost and won't retry.
UFUNCTION(Server, Reliable)
void ServerInteract(AActor* InFocus);
```



### 把逻辑挪到服务器RPC函数里

原本交互的代码在 `PrimaryInteract  `里，现在挪到新加的 `ServerInteract `里

客户端发起调用的时候要传参与哪个Actor互动，否则可能由于网络的问题一排`InteractActor`，客户端一边跑一边发就会出现互动滞后的情况，服务器用客户端传来的Actor来参与互动（这里目前还没有检测客户端传来的Actor的合法性等情况）

```cpp
void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}
void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor to interact.");
		return;
	}
	APawn* MyPawn = Cast<APawn>(GetOwner());
	ISGameplayInterface::Execute_Interact(InFocus, MyPawn);
}
```



执行射线检测的地方判断下是否是` IsLocallyControlled  `的，即是否是这段代码运行的那一端可以本地控制玩家而不是单纯从服务器同步下来的玩家移动（比如两个客户端在彼此的本地客户端内就不是`LocallyControlled` 的，在自己的客户端是`LocallyControlled`），这样可以减少本地客户端的不必要的射线检测

```cpp
APawn* MyPawn = Cast<APawn>(GetOwner());
if (MyPawn && MyPawn->IsLocallyControlled())
{
    FindBestInteractable();
}
```



### 修改`ASItemChest `使其能正确表现



#### 修改宝箱的Actor使其成为可被`Replicated`的

在 `ASItemChest `的构造函数里 设置这个Actor类的对象都是可被复制的

```cpp
SetReplicates(true);
```



#### 用 R`eplicatedUsing  `标识变量，当变量在服务器端变动时会自动复制给客户端，并执行绑定的函数

重写 `GetLifetimeReplicatedProps` ，添加要绑定的自动同步变量 `bLidOpened`，不用在头文件指定，因为`xxx.generated.h `里已经帮我们生成好了

服务器端会执行` Interact_Implementation` ， 修改`bLidOpened `的值，但是不会自动执行当 `bLidOpened `变化时执行的 `OnRep_LidOpened`，所以要手动显示调用

客户端当收到服务器下发的 `bLidOpened` 变化通知时会自动调用用UPROPERTY注册的 `OnRep_LidOpened`

```cpp
UFUNCTION()
void OnRep_LidOpened();

UPROPERTY(ReplicatedUsing = "OnRep_LidOpened", BlueprintReadOnly) // RepNotify
bool bLidOpened;



void ASItemChest::Interact_Implementation(APawn* InstigatorPawn)
{
	bLidOpened = !bLidOpened;
	OnRep_LidOpened();
}

void ASItemChest::OnRep_LidOpened()
{
	float CurrentPitch = bLidOpened ? TargetPitch : 0.0f;
	LidMesh->SetRelativeRotation(FRotator(CurrentPitch, 0, 0));
}

void ASItemChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASItemChest, bLidOpened);
}
```



#### GetLifetimeReplicatedProps 性能优化

考虑到CPU轮询的效率和减少同步时消耗的网络带宽

可以用` DOREPLIFETIME_CONDITION`，在要同步的成员变量后加参数的形式

`COND_OwnerOnly `只同步给拥有者，不会同步给其他client

`COND_InitialOnly `只在初始化的时候同步一次，适合最大生命值等不会变化的内容

```cpp
DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_InitialOnly);
```

