#  Assignment 2 (Blackhole Projectile, Dash Ability, Targeting) 



## 修复子弹发射方向不对的问题

有几个要注意的地方

1、射线检测是从相机的位置开始，但是计算新的发射方向的时候起始位置是手部的那个骨骼插槽而不是相机的位置

2、射线检测的终点要稍微长一点，一开始设置的1000感觉太短了



### FindLookAtRotation

```cpp
// https://zhuanlan.zhihu.com/p/108474984
FRotator FindLookAtRotation(FVector const& X)
{
	FVector const NewX = X.GetSafeNormal();
	FVector const UpVector = (FMath::Abs(NewX.Z) < (1.f - KINDA_SMALL_NUMBER)) ? FVector(0, 0, 1.f) : FVector(1.f, 0, 0);//得到原坐标轴的Z轴方向
	const FVector NewY = (UpVector ^ NewX).GetSafeNormal();//叉乘可得到Y'
	const FVector NewZ = NewX ^ NewY;//再次将X'与Y'叉乘即可得到Z'

	return FMatrix(NewX, NewY, NewZ, FVector::ZeroVector).Rotator();
}
```



```cpp
FRotator ASCharacter::CalcProjectileSpawnRotation(FVector HandLocation)
{
	FCollisionObjectQueryParams ObjectQueryParams; 
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FVector TraceBeginLocation = CameraComp->GetComponentLocation();
	FVector TraceEndLocation = TraceBeginLocation + GetControlRotation().Vector() * 5000;

	FRotator SpawnRotation;

	FHitResult Hit;
	bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, TraceBeginLocation, TraceEndLocation, ObjectQueryParams);
	if (bBlockingHit)
	{
		TraceEndLocation = Hit.ImpactPoint;
	}

	SpawnRotation = FindLookAtRotation(TraceEndLocation - HandLocation);
		// FRotationMatrix::MakeFromX(TraceEndLocation - HandLocation).Rotator();
	DrawDebugLine(GetWorld(), HandLocation, TraceEndLocation, FColor::Red, false, 2.0f, 0, 2.0f);
	return SpawnRotation;
}
```



## 加一个黑洞子弹

### 设置了生存时间但是没生效

重启蓝图点reset才行（



### 黑洞子弹的`RadialForceComponent`组件怎么也不产生效果

一开始是没有给放置的物件设置模拟物理

之后发现可能是Force设置的不够大，改成了-2000000

怀疑是蓝图和cpp冲突了，把定义在cpp里的组件删了改成蓝图重新创建一个就好了。。。



## 加一个子弹基类

### 处理player释放不同子弹

原本想用一个`TSubclassOf<ASProjectileBase> ClassToSpawn;` 来接收不同按键输入时修改成不同类型，但是总会出各种类数据成员初始化失败的问题，最后改成多个在Character的蓝图里先设置好的TSubClassOf，之后触发按键输入的时候直接赋值过去

```cpp
UPROPERTY(EditAnywhere, Category = "Attack")
TSubclassOf<ASProjectileBase> ClassToSpawn;

UPROPERTY(EditAnywhere, Category = "Attack")
TSubclassOf<ASProjectileBase> PrimaryAttackProjectile;

UPROPERTY(EditAnywhere, Category = "Attack")
TSubclassOf<ASProjectileBase> SecondAttackProjectile;
```



### 加的子弹不会移动

因为和角色的右手发生了碰撞，在蓝图加一下`ignore instigator`



### 加的子弹会受重力影响下坠

movement组件有个抛射物重力置为0 即可



## 重叠Actor时把子弹销毁

先修改Sphere的碰撞响应改成重叠

从碰撞检测的Sphere触发个重叠事件，只有设置了`simulating physics`的才会被销毁

![1696914495973](TyporaPic\1696914495973-1698308238269.png)



## Dash Projectile

碰撞组件设置碰撞类型、忽略Instigator的碰撞、设置碰撞事件OnHit











## 优化assginment2

### ProjectileBase

在UCLASS的宏里加了个参数`ABSTRACT`	表示该类是抽象类，不能被实例化

https://zhuanlan.zhihu.com/p/148098617



把子弹的碰撞体检测到命中的事件提到Base里了，之前是各写各的，magicprojectile在蓝图里，dashprojectile在cpp里，现在统一到projectilebase里



加了个粒子组件，用来表现命中时的粒子效果



### DashProjectile

原本的定时器理解错了，是发射抬手动画0.2s后生成子弹，子弹运动0.2s后停止，然后再过0.2s才触发传送

把碰撞检测命中改成派生基类的Explode方法，在这里写传送相关的逻辑

子弹运动0.2s后停止，要关掉粒子特效、移动组件的移动、关闭阻挡判断

 `TimerHandle_Detonate `定时器到时间触发 Explode 后要Clear掉，因为`DashProjectile`用了基类，Explode有两个触发函数，一个是Base的`OnHit`事件触发，另一个是`DashProjectile`的`BeginPlay`设置定时器到时间触发，所以为了防止子弹命中地面前后触发了两次 `Explode_Implementation`，要在这个函数开头位置给他清掉



### 发现一个bug 当摄像机朝天上时子弹会打地

原因：

原本射线检测的起始位置是摄像机的Component位置，当视角朝天的时候由于是第三人称，摄像机离角色还有一定距离，这个时候摄像机的Component位置其实在地下（，所以射线检测的障碍物是地面，子弹就从手发射到地面了233

```cpp
FVector TraceBeginLocation = CameraComp->GetComponentLocation();
FVector TraceEndLocation = TraceBeginLocation + GetControlRotation().Vector() * 5000;
```

修改：

改成起始位置是手那块骨骼的坐标，终点还是摄像机的终点，这样还是瞄哪打哪

```cpp
FVector TraceBeginLocation = HandLocation;
FVector TraceEndLocation = CameraComp->GetComponentLocation() + GetControlRotation().Vector() * 5000;
```

新的问题，子弹打的位置歪了，因为射线检测是从手到摄像机终点了

再改一版：

感觉问题出在虽然有弹簧臂但是摄像机的位置还是不太对，贴着墙或地面的时候还是会从墙的另一边向终点检测障碍结果把墙或者地面算成障碍物终点导致子弹方向不对

目前的技术力最后的解决办法是射线追踪的起点是摄像机的组件位置向前10个单位，情况大大缓解了

```cpp
FVector TraceBeginLocation = CameraComp->GetComponentLocation() + GetControlRotation().Vector() * 10;
FVector TraceEndLocation = TraceBeginLocation + GetControlRotation().Vector() * 5000;
```

根据调试后的结果的猜测：

摄像机本身占有体积，贴墙或贴地面的时候和墙/地面产生了碰撞，所以直接用组件位置会直接碰撞检测到墙/地面