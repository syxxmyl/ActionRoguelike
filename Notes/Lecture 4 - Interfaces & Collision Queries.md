# Lecture 4 - Interfaces & Collision Queries



## `BlueprintImplementableEvent` 和 `BlueprintNativeEvent` 函数说明符

https://zhuanlan.zhihu.com/p/588754236

### `BlueprintImplementableEvent `

`BlueprintImplementableEvent`：在C++可以声明函数（不能定义，蓝图重写），在C++里调用该函数，蓝图重写实现该函数

> 可以在**蓝图**或**关卡蓝图**中重载（overridden）函数。

当你想在蓝图中实现一个函数的逻辑而不是C++中实现的时候，可以使用这个关键字。你可以和`BlueprintCallable`关键字结合使用，以便可以从蓝图调用它，否则你只能在C++中调用。

```cpp
UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Character")
void SetupInventory();
```

 这里有个重要注意事项，在函数上指定无返回类型void 和指定某个返回类型（例如 bool），它们是有区别的。上面的代码示例类型为 void，因此生成一个 Event 节点。而下面的代码（以 bool 作为返回类型）中的函数是可以被重载的。这样的差异在其它关键字上也会出现。 

```cpp
UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Character")
bool SetupInventory();
```



### `BlueprintNativeEvent`

`BlueprintNativeEvent`：在C++可以声明和定义函数，在C++里调用该函数，蓝图重写实现该函数（蓝图可以重写或不重写C++父类函数）

> 此函数可以被蓝图重写，但同时也具有本地实现（译者：即可以有两份实现，蓝图实现和C++实现）。用这样的格式：**[函数名]_Implementation，**而不是 [函数名]；自动生成的代码会在必要时调用实现体。

当你想在C++代码中提供基本实现，并且在蓝图做逻辑扩展时，这个关键字会很有用。使用_Implementation 后缀（请参阅下面的代码）的函数体看起来和正常形式不太一样，你仍然像往常一样在代码中调用 [FunctionName] 而不带后缀。在这里，需要配合使用其它关键字，使用 `BlueprintCallable` 向蓝图公开代码。

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character")
void OnEnterInventory();
```

 下面的代码是写在 .cpp 文件中 

```cpp
void AMyGameCharacter::OnEnterInventory_Implementation()
{
    // 基础实现部分，可以在蓝图中进行扩充
}
```



## 加一个 `ActorComponent`

用来解耦逻辑代码，防止Character代码量过大，也可以更好的标识函数作用



## 用射线追踪碰撞检测来查找面前是否有Actor可以供Interact

`LineTraceSingleByObjectType` 来查找，如果面前的Actor没有设置对应的碰撞，那么就不会被检测到

```cpp
FCollisionObjectQueryParams ObjectQueryParams;
ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

AActor* MyOwner = GetOwner();

FVector EyeLocation;
FRotator EyeRotation;
MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
FVector End = EyeLocation + EyeRotation.Vector() * 1000;

FHitResult Hit;
bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLocation, End, ObjectQueryParams);
```



### 优化一下检测Actor的方法，改成Sweep球形探测

顺便加个break修一下会一次性全打开的bug

```cpp
TArray<FHitResult> OutHits;
FCollisionShape CollisionShape;
CollisionShape.SetSphere(30.0f);
GetWorld()->SweepMultiByObjectType(OutHits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, CollisionShape);
for (auto& Hit : OutHits) 
{
	AActor* HitActor = Hit.GetActor();
    if (HitActor)
    {
        if (HitActor->Implements<USGameplayInterface>())
        {
            APawn* MyPawn = Cast<APawn>(MyOwner);
            ISGameplayInterface::Execute_Interact(HitActor, MyPawn);
            break;
        }
    }
    
    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, Radius, 32, LineColor, false, 2.0f);
}
```



## 可互动的Actor执行继承的接口函数`Execute_Interact`

```cpp
AActor* HitActor = Hit.GetActor();
if (HitActor)
{
    // 判断是否为继承了Interface的类成员(这里是SItemChest)
    if (HitActor->Implements<USGameplayInterface>())
    {
        APawn* MyPawn = Cast<APawn>(MyOwner);
        // 执行接口
        ISGameplayInterface::Execute_Interact(HitActor, MyPawn);
    }
}
```



## 调试可视化起止点

`DrawDebugLine`

命中显示绿线，未命中显示红线

```cpp
FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;
DrawDebugLine(GetWorld(), EyeLocation, End, LineColor, false, 2.0f, 0, 2.0f);
```



## 给攻击加个动画

调用`PlayAnimMontage` 函数

```cpp
UPROPERTY(EditAnywhere, Category="PrimaryAttack")
UAnimMontage* PrimaryAttackAnim;


PlayAnimMontage(PrimaryAttackAnim);
```



## 给投射物加个延时生成

一般更好的解决方案是用动画完成通知或动画完成事件，这里还没接触到这些，就先用定时器

```cpp
GetWorldTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_TimeElapsed, ProjectileSpawnDelayTime);
```



用定时器有个小问题，如果在定时结束前又一次触发了则会覆盖掉生成投射物这个事件，因为用的定时器变量`TimerHandle_PrimaryAttack` 是同一个

表现上看就是连续按鼠标左键但是直到最后一次按`delaytime`后才生成了第一个投射物