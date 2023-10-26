# Lecture 15 - Console Variables, GameMode Rules

## 复活玩家

在`GameModeBase`里加个虚函数`OnActorKilled`，然后在`AttributeComp`里判断当受到的是伤害并且致死时调用它

`GameMode`只存在于Server端

### 用计时器委托绑定带参数的触发执行函数

用函数名的形式绑定， 执行函数要加 `UFUNCTION` 才能被识别到



### 复活传入的是Controller而不是Character

因为Character 已经死了，当执行复活的时候可能已经被删掉了

并且要执行`Controller->UnPossess();` 将Controller 和原本的 Character 解除绑定



```cpp
void ASGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	if (Controller)
	{
		Controller->UnPossess();
		RestartPlayer(Controller);
	}
}

void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, PlayerRespawnTimerInterval, false);
	}

	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim is %s, Killer is %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
}
```



### 复活以后UI的Health Bar有些问题

因为绑定发生在事件Construct 的时候， 复活以后绑的还是死掉的那个GamePlayer，所以会出问题

应该要加个玩家死亡和复活的`Event Broadcast`，然后在蓝图里重新绑一次



## 加一些控制台变量

```cpp
static TAutoConsoleVariable<bool> CVarDebugDrawInteraction(TEXT("su.DebugDrawInteraction"), true, TEXT("Enable Draw Debug Interaction"), ECVF_Cheat);
```



## 创建一个蓝图函数库

继承自 `UBlueprintFunctionLibrary`

引用类型在蓝图节点是输出引脚，用const修饰引用才是输入引脚



### 新增一个受到伤害会收到一个来自伤害方向的Impulse导致后退的函数

可以从`HitResult`里拿到方向、位置、命中哪块骨骼

```cpp
UFUNCTION(BlueprintCallable, Category = "Gameplay")
	static bool ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount, const FHitResult& HitResult);


bool USGameplayFunctionLibrary::ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount, const FHitResult& HitResult)
{
	if (ApplyDamage(DamageCauser, TargetActor, DamageAmount))
	{
		UPrimitiveComponent* HitComp = HitResult.GetComponent();
		if (HitComp && HitComp->IsSimulatingPhysics(HitResult.BoneName))
		{
			HitComp->AddImpulseAtLocation(-HitResult.ImpactNormal * 300000.0f, HitResult.ImpactPoint, HitResult.BoneName);
		}
		return true;
	}

	return false;
}
```



#### 命中了Bot但是没有出现效果

断点观察发现`HitResult`命中的是`AICharacter`的胶囊体而不是Mesh，由于现在还没有`MagicProjectile` 自己的碰撞通道，所以暂时只能先设置`AICharacter`的

```cpp
GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
GetMesh()->SetGenerateOverlapEvents(true);
```



## Bot死亡后在死亡的位置有碰撞体

### 查看碰撞

控制台指令` Show Collision `



### 修复

死亡的时候删除碰撞

```cpp
GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
GetCharacterMovement()->DisableMovement();
```



## 和物品交互射线检测是从人物的Eye而不是第三人称相机

因为射线检测的起始点是用` GetActorEyesViewPoint `获取的，而 起始点的Location 是用 `APawn::GetPawnViewLocation  `获取的，可以在Character 里 override 这个函数

这样起点就从人物模型的眼睛处改成了角色摄像机的位置

```cpp
virtual FVector GetPawnViewLocation() const override;


FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}
```



## 把创建viewport的蓝图节点从`BP_GamePlayer`挪到`BP_PlayerController`里

