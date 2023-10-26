# Lecture 26 – Animation Blueprints & UI Improvements

## 给character做一个眩晕的动画状态切换

### 向角色的动画蓝图里加动画状态切换

打开` Gideon_AnimBlueprint`，切换到`AnimGraph`页签，从`Locomotion`状态机进来可以看到有个Idle/Jogs，可以从这个地方开始做Stunned的动画状态切换

新建三个State节点分别是`StunStart`、`Stunned`、`StunStop`

从`Idle`到`StunStart` 用一个新的bool类型的变量`bIsStunned`来判断是否要执行

`StunStart`节点内直接用资源包里的 `Stun_Start `的动画播放就可以，注意细节栏里把循环动画的勾选给去掉

从`StunStart`到`Stunned `可以直接勾选细节栏里的 `Automatic Rule Based on Sequence Player in State`，即会自动根据起始节点动画的结束自动开始执行终点的动画

`Stunned`节点就直接用`Stun_Loop `就行，保持循环动画的选项

`Stunned `到 `StunStop `用`bIsStunned = false `的时候执行

`StunStop`用` Stun_Stop`，并关掉循环动画

`StunStop`到`Idle` 和`StunStart`到`Stunned `一样，勾选上`Automatic Rule Based on Sequence Player in State`就行

![1698134099116](TyporaPic\1698134099116.png)



### 做一个新的ActionEffect眩晕Stunned

在项目设置里加一个新的GameplayTag

做一个新的蓝图，继承自ActonEffect类，设置`Duration`、`Icon`、`GrantsTag`、`ActionName`



### 在cpp里通知Anim bIsStunned修改了

创建一个新的cpp类，继承自 `UAnimInstance`

创建一个bool类型的变量用来存放是否眩晕

创建一个`USActionComponent`的指针用来暂存Owner的`ActionComp`

需要重写两个函数，` NativeInitializeAnimation `是初始化函数，在这里拿到Owner的`ActionComp`

`NativeUpdateAnimation `类似Tick，在这里判断Owner现在是否还有`Stunned`状态

Stunned的`GameplayTag `直接在这里HardCode了

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
bool bIsStunned;

UPROPERTY(BlueprintReadOnly, Category = "Animation")
USActionComponent* ActionComp;

void NativeInitializeAnimation() override;

void NativeUpdateAnimation(float DeltaSeconds) override;




void USAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	AActor* OwningActor = GetOwningActor();
	if (OwningActor)
	{
		ActionComp = USActionComponent::GetActions(OwningActor);
	}
}

void USAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	static FGameplayTag StunnedTag = FGameplayTag::RequestGameplayTag("Status.Stunned");
	if (ActionComp)
	{
		bIsStunned = ActionComp->ActiveGameplayTags.HasTag(StunnedTag);
	}
}
```



### 替换Gideon_AnimBlueprint 里的 Parent Class

在动画蓝图的类设置里修改父类为刚才创建的`SAnimInstance`，ue5会自动替换掉之前在蓝图里创建的bool类型的和cpp代码重复的`bIsStunned`，但是视频里的ue4.25还不会，所以要手动右键查看引用，然后在蓝图查看器里勾选上显示继承的变量，把cpp里的那个bool类型的全部替换上去





## 给character做一个奔跑的动画状态切换

### 创建一个混合空间BlendSpace

根据两个坐标轴不同的值进行不同动画的切换

目前只要用到一个参数速度，所以创建一个`BlendSpace1D `的动画蓝图`Sprinting_BS_1D`即可

设置水平坐标轴的相关内容

![1698137199374](TyporaPic\1698137199374.png)



然后在坐标轴中设置0,400,600三个采样点，0使用Idle，400和600都用`Fwd`，区别是600的右击设置了Rate scale 为1.5，这样看起来更快



![1698137251341](TyporaPic\1698137251341.png)



#### 让采样点之间的切换更平滑

设置取样平滑的权重速度 `Target Weight Interpolation Speed Per Second `，0是不插值，可以填写0-10之间的值

![1698138174174](TyporaPic\1698138174174.png)





### 使用混合空间修改玩家动画

#### 创建一个Locomotion

用前面刚做好的`Sprinting_BS_1D`

![1698137419120](TyporaPic\1698137419120.png)



#### 修改Locomotion的Idle/Jogs State

`Blend Pose by Bool`，新建一个bool类型的变量`bIsSprinting`

![1698137567629](TyporaPic\1698137567629.png)



如果要让修改姿势切换更平滑，可以增加混合时间的大小



#### 在事件列表里给bIsSprinting赋值

前面一个眩晕动画是在cpp里做的，这里换种方式用蓝图

![1698137757754](TyporaPic\1698137757754.png)





## 提升可互动物品的UMG弹窗的交互性

对于不同的交互物品显示不同的文字

### 在InteractActor里加个新的函数，获取交互文本

在SGameplayInterface 接口类里加

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
FText GetInteractText(APawn* InstigatorPawn);
```

在继承了他的子类里一路重写下去

PowerUpActor 里提供一个空Text的返回值作为基类的返回

```cpp
FText GetInteractText_Implementation(APawn* InstigatorPawn);


FText ASPowerUpActor::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	return FText::GetEmpty();
}
```

HealthPotion 里根据血量的多少显示不同的文本，满血提示不能加血

可以用 `LOCTEXT_NAMESPACE` 宏指定本地化文本的Namespace，在cpp的开头define，在cpp的结尾undef

用`FText::Format` 格式化Text类型的文本

```cpp
virtual FText GetInteractText_Implementation(APawn* InstigatorPawn) override;




#define LOCTEXT_NAMESPACE "InteractableActors"

FText ASPowerUpActor_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
	if (AttributeComp && AttributeComp->GetHealthMax() == AttributeComp->GetHealth())
	{
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health.");
	}
    
	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} credits. Restore {1} health."), ConsumeCreditAmount, AddHealthAmount);
}

#undef LOCTEXT_NAMESPACE
```



### 修改Default Interaction Widget 

把原本的文本包裹进垂直框，然后加一个新的文本框` InteractionMessageText `设为变量

![1698138681102](TyporaPic\1698138681102.png)



在事件蓝图里每Tick修改一次文本框的内容，内容就从上面新加的`GetInteractText`里取

![1698140046877](TyporaPic\1698140046877.png)



### 美化Default Interaction Widget

给垂直框加个Border边界包裹，然后设置边界的Background

![1698140349592](TyporaPic\1698140349592.png)



### 处理文本为空仍然占位的情况

在事件蓝图里check一下Text是否为Empty，如果是Empty的话就把文本框折叠了

![1698140714621](TyporaPic\1698140714621.png)

