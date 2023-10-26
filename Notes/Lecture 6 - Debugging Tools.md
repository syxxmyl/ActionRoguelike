# Lecture 6 - Debugging Tools

## `URadialForceComponent` 组件

### `Constant Force`和`Impulse`

https://docs.unrealengine.com/4.27/zh-CN/Resources/ContentExamples/Physics/1_3/



### `auto-active`为什么要关掉

`URadialForceComponent`的`TickComponent` 函数会每帧检查如果已激活，就会用球形探测查找周围actor然后施加`Constant Force`，不过这个力很小，但是最好给他关掉



## 什么时候绑定事件函数比较好

在构造函数中绑定函数可能会由于ue的热加载机制等问题而无法成功绑定，所以可以挪到下面组件初始化完毕再绑定

`PostInitializeComponents` 在 `Constructor` 和 `BeginPlay` 之间

```cpp
void ASExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MeshComp->OnComponentHit.AddDynamic(this, &ASExplosiveBarrel::OnActorHit);
}
```



## 添加调试信息

https://nerivec.github.io/old-ue4-wiki/pages/logs-printing-messages-to-yourself-during-runtime.html

TEXT宏用来处理字符集的相关问题，如果要带参数，记得`%s` 类型的后面参数要加一个`*`

```cpp
UE_LOG(LogTemp, Log, TEXT("OnActorHit a barrel 爆炸"));

UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s, at game time: %f"), *GetNameSafe(OtherActor), GetWorld()->GetTimeSeconds());

FString CombinedString = FString::Printf(TEXT("Hits at location: %s"), *Hit.ImpactPoint.ToString());
DrawDebugString(GetWorld(), Hit.ImpactPoint, CombinedString, nullptr, FColor::Green, 2.0f, true);
```



### `ensure` 和 `check`

ensure和check都是用来判断statement是否为真的，区别是ensure判断为否的时候只会切回调试一次，后续会忽略，游戏仍能继续进行，而check会直接终止这个游戏进程

`ensureAlways` 可以一直触发切回调试

`ensure`在打包的时候会视为不存在，不会影响到成品游戏



### vs调试有问题

装个官方插件

https://docs.unrealengine.com/5.3/en-US/using-the-unrealvs-extension-for-unreal-engine-cplusplus-projects/



## 移动玩家身上的摄像机

直接移动摄像机会导致弹簧臂组件的碰撞检测失效，所以要移动的其实是弹簧臂的长度和插槽偏移(socket offset)



## 加个umg准星

准星暂时用一个空白的image填充

在Character的蓝图里 `BeginPlay Even`t->`Create Widget`->`Add to Viewport`




