# Lecture 7 - UMG & (Player) Attributes

## 增加一个属性`ActorComponent`

### `UPROPERTY`常见的不同说明符的含义

```
EditAnyWhere			edit in BP editor and per-instance in level
VisibleAnyWhere			`read-only` in BP editor and level (use for components)
EditDefaultsOnly		edit in BP editor and hide variable per-instance
VisibleDefaultsOnly		`read-only` in BP editor only
EditInstanceOnly		edit in per-instance only

BlueprintReadOnly		`read-only` in BP scripting
BlueprintReadWrite		`read-write` access in BP scripting

```



## 给子弹加一个重叠事件

```cpp
SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap());
```

`OnActorOverlap`的参数可以从`OnComponentBeginOverlap` 一路跟到定义找到，然后截取`OnComponentBeginOverlap`之后的参数内容去掉类型和变量名之间的逗号即可



### `OnActorOverlap` 一定要加`UFUNCTION()`对蓝图可见



### 当子弹和有`AttributeComp`组件的Actor重叠时修改该Actor的Health

```cpp
void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ensure(OtherActor))
	{
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));
		if (ensure(AttributeComp))
		{
			AttributeComp->ApplyHealthChange(-20.0f);

			Destroy();
		}
	}
}
```



## FString FName FText

**FString**	常用于debug，处理字符串操作如append、split等

**FName**	比FString 有更好的Hash比较效率，常用于系统和游戏逻辑中，一旦创建实例便无法再修改值

**FText**		在UI中显示，可以轻松文本本地化



## 加一个UMG显示玩家血量

要注意的是游戏开始时玩家可能还没有初始化好，这个时候显示血量的文本要判断下玩家是否为Nullptr，如果是空则不要传值到显示界面

![1696927793797](TyporaPic\1696927793797.png)



## 改进UMG显示逻辑

现在会每帧都执行ui界面的整套逻辑，效率太低了，可以改成事件驱动ui变化

https://docs.unrealengine.com/4.27/zh-CN/InteractiveExperiences/UMG/HowTo/EventBasedUI/



### 在cpp代码里加一个事件

类似之前的ComponentHit之类的，可以借助UE的宏实现自定义的事件

`UPROPERTY(BlueprintAssignable)`  借助这个可以在蓝图中类似OnComponentHit这样显示出事件

```cpp
// 定义事件类型
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewHealth, float, Delta);

// 实例化事件
UPROPERTY(BlueprintAssignable)
FOnHealthChanged OnHealthChanged;

bool USAttributeComponent::ApplyHealthChange(float Delta)
{
	Health += Delta;
    // 通知订阅了该事件的其他物件，比如蓝图等
	OnHealthChanged.Broadcast(nullptr, this, Health, Delta);
	return true;
}
```



### 在UMG里连蓝图

当这个UMG 触发构造的时候把这个UI Assign到上面自定义的OnHealthChanged 事件上，每次当这个事件触发时更新进度条

![1696929975540](TyporaPic\1696929975540.png)

##### 一个trick，可以获取类内成员的默认值当做除数，这里是用Health暂时代替HealthMax



#### `Event Pre Construct` 和 `Event Construct` 区别

`Event Pre Construct`	在加入到Viewport前就会触发，并且即使没有开始游戏，在编辑器编辑界面的时候也会运行，可以改一些基础设置如颜色、字体大小等

`Event Construct`	当UI准备完成加入到Viewport的时候才触发



### 给Text文本加个小动画

在UMG下面的动画新建一个`PulseHealthAnim`

然后移动时间轴，去文本框的`Render Transform`  修改Scale的值

再点击Scale 旁边的Add property key frame 加到动画序列里，在动画的开始和结束帧设置Scale为1.0，中间设置一个1.2的Scale，表现就是放大再恢复了

然后把他加到上面的`OnHealthChanged`事件触发的蓝图里连上`Play Animate`



#### 播放完动画发现文本框好像变大了

因为做动画的时候修改了文本框的Scale，点击动画旁边的扳手->`Restore Pre-Animated State` 来恢复到做动画前的状态，然后点击Scale旁的回撤箭头来返回到原本的状态