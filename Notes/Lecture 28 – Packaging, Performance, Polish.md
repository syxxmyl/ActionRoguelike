# Lecture 28 – Packaging, Performance, Polish

## 打包要做的事情

### 打包前在设置里选择要打包哪些地图

![1698205253415](TyporaPic\1698205253415.png)



### 打包提示AssetManger重复了

用的5.1的没出现这个问题，记录一下视频里的解决方案

在4.25的引擎版本会在引擎的`Config/BaseGame.ini `和项目自己的 `DefaultGame.ini `里都产生一份`Map `和` PrimaryAssetLabel`的 AssetType，因为是先引入引擎的再引入项目自己的，所以可以在项目自己的`DefaultGame.ini`里加一行

```
!PrimaryAssetTypesToScan=ClearArray
```

即可



### 打包出来的版本没有怪物

项目设置里AssetManager 需要设置一下前面加的Monsters 的烘焙规则为固定烘焙



## 性能优化

### 一些小技巧

1、尽量避免用Tick，转而使用事件通知回调或定时器回调，尽量不要每帧都检测，可以每5帧、每10帧检测一次

2、对于可以移动的Actor要注意碰撞设置

​	`bGenerateOverlapEvents` 设置为`false`，只有需要的时候才设置为`true`，否则每帧都会调用它查询周围的环境信息

​	`Collision Profiles `碰撞配置只配置需要做出反应的那些碰撞通道，小物件或不会碰到的物体可以直接设为`Disable`

​	除非要使用物理模拟`Physics Simulation`，否则`Collision Query`碰撞查询只选用` Query Only` 选项

3、尽量避免`Hard Reference `到其他内容，不但影响游戏性能，而且在开发期也会影响编辑器加载时间和蓝图编译时间

​	可以使用`BaseClasses`，`Interfaces`，`FNames`，`GameplayTags` 等避免直接用具体的某个类，从而来减少`Hard Reference`

​	

### cpp和蓝图的比较

cpp性能会更好，管理起来会更方便

一些蓝图运行比较慢的逻辑可以迁移到cpp里



### stat 控制台指令

可以展示出各个模块的使用cpu的情况、也可以统计各种类型的数据

https://docs.unrealengine.com/5.1/zh-CN/stat-commands-in-unreal-engine/



#### 制作自定义的stat统计 如统计 StartActionByName 函数



在ActionRouguelike.h 项目的头文件里定义StatGroup

```cpp
DECLARE_STATS_GROUP(TEXT("ActionRoguelike_Game"), STATGROUP_ACTIONROGUELIKE, STATCAT_Advanced);
```



在ActionComponent.cpp里用这个`StatGroup `声明一个关于`StartActionByName`的新的统计项，然后在函数里调用它开始计数

```cpp
DECLARE_CYCLE_STAT(TEXT("StartActionByName"), STAT_StartActionByName, STATGROUP_ACTIONROGUELIKE);


bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	SCOPE_CYCLE_COUNTER(STAT_StartActionByName);

	// ...
}
```



### Unreal Insights 性能分析

https://docs.unrealengine.com/5.1/zh-CN/unreal-insights-in-unreal-engine/



#### 加一个自定义的统计标签

比如标记StartAction是在哪里开始的

```cpp
bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	// ...
	TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));

	Action->StartAction(Instigator);  
}
```



## 修复一些之前项目的Warning警告

编辑器设置里打开 异常时中断蓝图 和 PIE时提升输出日志警告 两个勾选

### anim_blueprint 有除0操作

event graph开头的地方有用`tick delta `做除数的地方，检查下只有大于0才执行即可



### 用 `bReplicates = true;  `代替` SetReplicates(true); `

只有构造函数以外的地方才用`SetReplicates`



### EffectSlot 一些Duration为0 的会报除0操作

检查下只有大于0才执行



### EffectSlot 软对象引用为空

构造的地方Icon是异步加载的，如果有些`ActionEffect`没有设置`Icon`的资产就会出现这个问题，检查下只有软对象引用有效才执行后面的内容



### BurningEffect的Duration如果为0 当Instigator死亡进入Pending Kill的时候蓝图调用会出错

因为Burning的伤害逻辑是写在蓝图里的，如果主体Actor已经死亡进入`Pending Kill`状态，而这个时候Duration的定时器又触发了执行逻辑，就会出问题

修改一下`ActionComponent`，重写`EndPlay`函数，当结束的时候遍历下`Actions`如果还有在`Running`的就停掉

```cpp
virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

void USActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<USAction*> ActionsCopy = Actions;
	for (USAction* Action : ActionsCopy)
	{
		if (Action && Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}
	Super::EndPlay(EndPlayReason);
}
```



