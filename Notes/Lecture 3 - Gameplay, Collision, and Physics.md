# Lecture 3 - Gameplay, Collision, and Physics



## Controller 

不会显示在World中，但是是Player和AI的移动基础

有`ControlRotation`，常用于摄像机，和`PawnRotation` 是两个互相隔离的概念

`PlayerController`	每个`player`都有自己的`controller`，用于处理玩家输入，生命周期不和角色的生死绑定

`AIController`	以逻辑驱动AI的移动





## 让人物前进的移动方向和摄像机朝向一致

只需要按w往前移动的同时旋转摄像机，就可以自由切换移动朝向，像魂类游戏一样

```cpp
void ASCharacter::MoveForward(float value)
{
    // 获取控制器朝向
	FRotator ControlRot = GetControlRotation();
	// 防止输入值异常导致人试图移动到地下或天空
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

    // 
	AddMovementInput(ControlRot.Vector(), value);
}
```



### 只在设置里和代码里设置了鼠标Y的`LookUp`事件，为什么没效果

因为蓝图里弹簧臂组件的-摄像机设置->使用Pawn控制旋转默认是关闭的，即默认是把摄像机和角色分离开的

可以直接在蓝图操作，也可以在构造函数的地方写代码打开，因为蓝图里默认是关闭的每个蓝图	都要单独再设置一次

```cpp
SpringArmComp->bUsePawnControlRotation = true;
```



### `bUseControllerRotationYaw` 和 `charactermovement` 的`bOrientRotationToMovement`

`bOrientRotationToMovement` 用于设置玩家移动时是否转朝向为移动方向

`bUseControllerRotationYaw` 用于设置 组件的正朝向是否与控制器`PlayerController`的朝向一致

https://zhuanlan.zhihu.com/p/144610256

https://www.bilibili.com/read/cv19334054





## 发射子弹

创建物体都是由World来执行的

```cpp
void ASCharacter::PrimaryAttack()
{
	// 获取骨骼插槽为"Muzzle_01"的坐标，这样子弹就不是从玩家中心点发射
	FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
	FTransform SpwanTM = FTransform(GetControlRotation(), HandLocation);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ASMagicProjectile>(ProjectileClass, SpwanTM, SpawnParams);
}
```



### 设置子弹初始化生成位置

去骨骼里看下右手的那块骨骼插槽叫什么，然后在cpp里拿到坐标即可

```cpp
// 获取骨骼插槽为"Muzzle_01"的坐标，这样子弹就不是从玩家中心点发射
FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
```



### 设置子弹的碰撞类型

可以在蓝图中设置，也可以在cpp代码中设置

```cpp
// 只对Pawn的Overlap事件有反应
SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

// 设置碰撞类型为WorldDynamic
SphereComp->SetCollisionObjectType(ECC_WorldDynamic);
```



也可以在项目设置里指定新的碰撞类型，比如`Projectile` 后在代码设置新的类型

```cpp
SphereComp->SetCollisionProfileName("Projectile");
```



## 加一个爆炸油桶类

继承自`AActor`，根组件为`UStaticMeshComponent`，爆炸的物理效果由`URadialForceComponent` 提供

```cpp
UPROPERTY(VisibleAnywhere)
UStaticMeshComponent* StaticMesh;

UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
URadialForceComponent* RadialForce;
```

去蓝图里编辑 `URadialForceComponent`，爆炸半径、冲量大小等

去蓝图里编辑 `UStaticMeshComponent` ，触发组件命中的事件后执行 `Fire Radial`，`Target`是自己身上的那个`Radial Force`，即可产生子弹命中油桶后爆炸把附近的Actor冲飞的效果

