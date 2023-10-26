# Lecture 20 – Multiplayer 2

## 修改BP_PlayerController创建Main_HUD的内容

因为Controller 只存在于服务器和自己本地客户端，其他客户端不会有自己这个Character的Controller

而服务器启动的时候会运行多次Controller的`BeginPlay`，只有一次是服务器自己的Character的Controller能被正确调用

类似上面修改`InteractionComponent`的每Tick一次的`LineTrace`一样，在创建Hud前判断一下当前运行的实例是否为Local Controller

![1697618957509](TyporaPic\1697618957509.png)



## 之前做的根据EQS查询的结果在游戏开始时随机生成若干个PowerUpActor的内容客户端看不到

因为`PowerUpActor` 还没有

```cpp
SetReplicates(true);
```



### 攻击发射的子弹也看不到

因为`ProjectileBase` 还没有

```cpp
SetReplicates(true);
```

并且如果只改上面一个地方，只有服务器发射的子弹能让客户端看到，客户端发射的子弹还是不能让其他客户端和服务器看到，因为Replicate只能从服务器到客户端

后面要修改发射的Action逻辑支持多人的情况才能彻底解决



## 正确复制`AttributeComp`的变化同步

核心思路是属性的变化放在服务器端来做，把结果同步给客户端，然后双端一起执行结果的表现

### 给属性成员加上` Replicated ` 标签

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="Attributes")
float Health;
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Attributes")
float HealthMax;
```



### 构造函数指定同步方式

`SetIsReplicatedByDefault` 用来指定组件Component的复制方式，不会考虑这个组件的Parent是否复制同步

```cpp
SetIsReplicatedByDefault(true);
```



### 用Multicast 同步`OnHealthChanged`事件

因为当前生命值Health已经通过`Replicated`标签同步给客户端了，所以即使`OnHealthChanged`事件没同步过去也不会影响逻辑

```cpp
UFUNCTION(NetMulticast, Reliable) // 以后修改Character受到伤害死亡的时候再把这个地方改成Unreliable
void MulticastHealthChanged(AActor* InstigatorActor, float NewHealth, float Delta);
```



#### `NetMulticast` 和 `ReplicatedUsing` 的区别

比如一个玩家已经开了若干个箱子，这时另一个玩家进入了，如果每个箱子的打开状态是

`NetMulticast  `更适合事件的变化，那对于第二个玩家进入前的那些箱子的状态都是未打开，更倾向于关注发生事件的这一刻要处理的内容

`ReplicatedUsing` 更适合表示状态的变更，那对于进入前的箱子的状态会同步成打开，更倾向于关注这个状态的变更带来的要处理的内容，至于是什么时候变更的不是特别在意



#### Multicast + Reliable 会同步给所有客户端

https://zhuanlan.zhihu.com/p/483485488

提到后来修改了这块，如果不满足网络相关性Relevancy 则不管是不是Multicast都不会发



### 把原本的`OnHealthChanged`的`Broadcast`挪到`MulticastHealthChanged`里

这样服务器在执行`MulticastHealthChanged`的时候就会正常衔接后面的逻辑内容，客户端收到广播也会执行`OnHealthChanged`保持表现效果和单人的时候一致



```cpp
// OnHealthChanged.Broadcast(InstigatorActor, this, Health, RealDelta);
if (RealDelta != 0.0f)
{
    MulticastHealthChanged(InstigatorActor, Health, RealDelta);
}



void USAttributeComponent::MulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}
```



## 给Character头上也显示个血条

复用之前的`MinionRange_Health`的UMG，把`AttachedActor`提升权限到`BlueprintReadWrite`，借助`meta = (ExposeOnSpawn=true)` 让其在Spawn的时候暴露出来成为一个参数

```cpp
UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (ExposeOnSpawn=true))
AActor* AttachedActor;
```



在Character的`BeginPlay`蓝图连一下`create widget`

![1697628216453](TyporaPic\1697628216453.png)





## 蓝图中处理网络同步

以之前的爆炸桶为例

在蓝图里可以勾选`Replicated`

![1697630383767](TyporaPic\1697630383767.png)

### 只想在服务器中执行某些内容

用`Switch Has Authority `或者 `Has Authority + Branch` (Switch Has Authority的实现就是这两个结合)，如果为true表明是服务器在执行

![1697630220349](TyporaPic\1697630220349.png)



### 定义自定义事件的复制类型及是否可靠送达

创建自定义事件后在细节栏里有相关内容的勾选

![1697630293949](TyporaPic\1697630293949.png)



### 尽量使服务器和客户端的效果表现同步

`Fire Impulse` 和设置隐藏等都放在RPC里，这样两端都会执行

服务器执行`Multicast`后用 `Set Life Span `续两秒，然后再执行销毁，因为立刻销毁会导致客户端还没有执行RPC，这个Actor已经随着服务器的销毁而终止，也就不会执行RPC的内容了

![1697630334069](TyporaPic\1697630334069.png)



## AI在同步方面的内容

AI只会在Server端跑，因为AI需要`AIController`来执行行为树，而`AIController`只会出现在Server端



## ActionComponent的同步复制

先把`StartAction`挪到服务器端，设置为`Reliable`

当`StartActionByName`找到要执行的Action时，根据`GetOwner()->HasAuthority()` 判断这一端是客户端还是服务端，如果是客户端的话就调用`ServerStartActionByName`的RPC，然后再执行后续

```cpp
UFUNCTION(Server, Reliable)
void ServerStartActionByName(AActor* Instigator, FName ActionName);



void USActionComponent::ServerStartActionByName_Implementation(AActor* Instigator, FName ActionName)
{
	StartActionByName(Instigator, ActionName);
}

bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			//...

			if (!GetOwner()->HasAuthority())
			{
				ServerStartActionByName(Instigator, ActionName);
			}

			Action->StartAction(Instigator);  
			return true;
		}
	}

	return false;
}
```



