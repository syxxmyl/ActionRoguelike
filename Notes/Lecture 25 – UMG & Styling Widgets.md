# Lecture 25 – UMG & Styling Widgets

## 在屏幕上显示Bot身上受到的Debuff的图标

### 制作Debuff图标 EffectSlot UMG

一个image组件即可



### 制作Debuff栏 EffectContainer UMG

用一个Horizontal Box即可，往里面随便拖几个EffectSlot进去，然后要记得把这个水平框设置为变量，因为要在cpp里用它来控制EffectSlot的增删



### 把EffectContainer 加到 MinionHealth UMG上

原本的`HealthBarImage`用一个垂直框包起来，然后在这个垂直框下加一个`EffectContainer`



### 设置哪些Action可以被监听并显示在Debuff栏上

在EffectContainer里新建一个自定义事件`SetActionComponent`，输入参数加一个`SActionComponent`类型的`ActionComp`

然后去MinionHealth里在构造事件里把这个Widget的`AttachedActor`的`ActionComponent`绑定到`EffectContainer`上

![1698054724112](TyporaPic\1698054724112.png)



### 在cpp里添加ActionStart和ActionStop的事件

在`ActionComponent`里添加事件委托，然后在Action的`Start`和`Stop`里广播事件

因为有些`ActionEffect`是AutoStart不经过`ActionComponent`的，所以在Action自己类内广播

把`ActionComponent`的Actions 的`TArray` 暴露给蓝图

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, USActionComponent*, OwningComp, USAction*, Action);

UPROPERTY(BlueprintAssignable)
FOnActionStateChanged OnActionStarted;

UPROPERTY(BlueprintAssignable)
FOnActionStateChanged OnActionStopped;

UPROPERTY(BlueprintReadOnly, Replicated)
TArray<USAction*> Actions;



void USAction::StartAction_Implementation(AActor* Instigator)
{
	// ...
	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}
void USAction::StopAction_Implementation(AActor* Instigator)
{
	// ...
	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}
```



### 在EffectContainer的事件蓝图里绑定ActionStart事件

每次事件触发的时候就把一个EffectSlot添加到命名为`EffectsBox` 的SizeBox里，并且在绑定后就调用一次把当前`ActionComponent`里的Actions依次执行一次`OnActionStarted`保证UI的正确性

![1698055659430](TyporaPic\1698055659430.png)



#### 过滤一下Action，只显示ActionEffect

![1698055894051](TyporaPic\1698055894051.png)



#### EffectSlot加个ActionEffect类型的变量，这样可以拿到持续时间之类的信息



![1698056118868](TyporaPic\1698056118868.png)

在EffectContainer里把引脚连上

![1698056187706](TyporaPic\1698056187706.png)



#### 在Action里加个成员暴露给蓝图，这样每次加EffectSlot的时候能根据ActionEffect的不同类型添加不同的Image

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
UTexture2D* Icon;
```



在EffectSlot的事件里判断下Action的Icon是否有效，有效的话就设置Image

![1698059019053](TyporaPic\1698059019053.png)



### 在EffectSlot的事件蓝图里绑定ActionStop事件

绑定下`OnActionStopped`，判断下Stopped的`Action`是不是自己这个EffectSlot绑定的，如果是的话就`Remove From Parent`

![1698059398376](TyporaPic\1698059398376.png)



#### Debuff标记移除的时候血条会上移

因为`MinionHealth_Widget`里的SizeBox的`Alignment`的y偏移是0.5，当上部的EffectSlot消失后会为了对齐而移动，y偏移改成1就行了



### 做一个Buff的Material

可以从HealthBar的那个Material复制一个改一改

`LinearGradient `改成用`VGradient`，并且翻转一下

`ProgressAlpha `调整为`0.75`，这样可以看到默认的两端的效果

`ColorBottom`和`ColorTop`表示两端的颜色，都是转换为参数的Vector3

加一个`TextureSample`，用它的Alpha通道输出值作为最终结果的不透明度，并把材质的混合模式从不透明改成半透明

![1698059941010](TyporaPic\1698059941010.png)



#### 在EffectSlot里修改Image并提供Icon参数

![1698060297802](TyporaPic\1698060297802.png)



### 在cpp里添加ActionEffect的剩余时间

考虑到多人游戏时的同步性，在Action基类里加一个float类型的`TimeStarted`，设置为可复制的，然后当Action开始的时候判断下如果Owner是服务端有权限的，就给他赋值

```cpp
UPROPERTY(Replicated)
float TimeStarted;

void USAction::StartAction_Implementation(AActor* Instigator)
{
	// ...
	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}
	// ...
}

void USAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// ...
	DOREPLIFETIME(USAction, TimeStarted);
}
```



在ActionEffect里计算还剩多久

```cpp
UFUNCTION(BlueprintCallable, Category = "Action")
float GetTimeRemaining() const;

float USActionEffect::GetTimeRemaining() const
{
	float RemainingSecond = Duration;
	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (GameState)
	{
		float EndTime = Duration + TimeStarted;
		if (EndTime - GameState->GetServerWorldTimeSeconds() > 0.0f)
		{
			RemainingSecond = EndTime - GameState->GetServerWorldTimeSeconds();
		}
	}
	return RemainingSecond;
}
```



### 在EffectSlot的蓝图里根据剩余时间每Tick更新ProgressAlpha

`Tick Event `->` Get Dynamic Material` -> `Set Scalar Parameter Value`

![1698061183179](TyporaPic\1698061183179.png)



## 美化一下UI

### 给上面加的BuffSlot 加一个底部背景，这样在白色背景后看着会好点

用一个Overlay包裹Image，然后再加一个Image移到EffectImage图层的下面

![1698122672802](TyporaPic\1698122672802.png)



### 修复菜单按钮的材质拉伸问题

因为原本的材质是个正方形，而按钮是个扁长方形，材质会从中间向四周拉伸导致出现了窄边间隙宽 长边间隙窄的问题

把MenuButton的绘制为`Draw As`改成盒体，然后设置边缘长度为0.1

![1698123907232](TyporaPic\1698123907232.png)



### 给菜单界面加一个背景模糊(Background Blur)的效果

设置模糊强度为2.0，锚点更新为整个界面，偏移全部为0

不要模糊菜单界面本身，所以往上挪到垂直框的上面

![1698123521222](TyporaPic\1698123521222.png)



### 给菜单按钮加一个背景图

和上面BuffSlot 的处理方式类似，加一个`Overlay`然后加一个`Image`，修改Image的绘制方式为盒体并设置边缘长度已防止材质拉伸

![1698124068674](TyporaPic\1698124068674.png)



### 给小兵的血量条加一个背景图并处理材质拉伸的问题

在`M_HealthBar`的Material里加个`TextureSample`并Add 到最终颜色上

![1698124334736](TyporaPic\1698124334736.png)



由于小兵的血条的这张图片已经是拉伸过长宽不一致的了，所以无法把图片再次渲染成所需，所以只用上面的设置为盒体然后设置一个边缘长度并不能解决问题

![1698124698857](TyporaPic\1698124698857.png)

需要在这个Image外面包一层SizeBox尺寸框，然后会发现边缘就和之前的一样处理好了，然后设置这个SizeBox的子布局的所需宽度长度即可

![1698124673086](TyporaPic\1698124673086.png)

![1698124691871](TyporaPic\1698124691871.png)



