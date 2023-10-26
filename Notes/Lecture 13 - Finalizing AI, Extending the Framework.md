# Lecture 13 - Finalizing AI, Extending the Framework

## 哪些组件要`SetupAttachment`

继承自`USceneComponent`的需要Setup

继承自`UActorComponent`的不需要



## AI被击杀的设置

### 停止行为树

```cpp
// stop BT
AAIController* AIController = Cast<AAIController>(GetController());
if (AIController)
{
    AIController->GetBrainComponent()->StopLogic("Killed");
}
```



### 更新碰撞

启用身体的物理模拟，这样会受到重力倒地		代替倒地动画

修改碰撞类型，让他不会穿过地面一直向下

```cpp
// set ragdoll
GetMesh()->SetAllBodiesSimulatePhysics(true);
GetMesh()->SetCollisionProfileName("Ragdoll");
```



### 设置存在时间

到时间后消失

```cpp
SetLifeSpan(10.0f);
```



## 当AI受到攻击时也会将攻击者设为`TargetActor`而不只是依靠`UPawnSensingComponent`

改一下`AttributeComp`的`ApplyHealthChanged`，加个释放者的`AActor`指针



## 添加一个获取`AttributeComp`的静态函数

一般的情况下需要参考 `UGameplayStatics` ，继承自 `UBlueprintFunctionLibrary`

现在先简单的放在`AttributeComponent`里

```cpp
UFUNCTION(BlueprintCallable, Category = "Attributes")
	static USAttributeComponent* GetAttributes(AActor* FromActor);




USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<USAttributeComponent>(FromActor->GetComponentByClass(USAttributeComponent::StaticClass()));
	}

	return nullptr;
}
```



## 让游戏时间流速加快

按``  ` 打开控制台，在控制台输入 `Slomo x `		x为倍数



## 如果AI射击的`TargetActor `已经死亡，就不要再对着尸体射击了

正常应该是加个`BT_Service`来处理这件事，简单的修改的话就是在`BT_Task_RangedAttack` 里判断下如果已经死了就返回  `EBTNodeResult::Failed `



## 让AI的射击准确性降低一些

用随机数修改AI生成子弹的Rotation

```cpp
MuzzleRotation.Pitch += FMath::RandRange(0.0f, MaxBulletSpread);
MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletSpread, MaxBulletSpread);
```

