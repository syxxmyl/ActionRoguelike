# Lecture 14 - UMG With C++ & More Framework Extensions

## UPROPERTY在垃圾回收方面的作用

用`UPROPERTY`指定的变量会由UE系统负责垃圾回收，因此在其他组件引用的`AActor`如果被销毁了会自动置为`nullptr`，不需要额外其他的判断



## 当怪物受到伤害时在屏幕上显示他的血条

### 创建一个`WorldUserWidget`作为后续UMG的基类

继承自`UserWidget`

刷新用的是`NativeTick`

会在`.uproject`里加一个新的UMG Module来编译

在cpp里设置展示UI到屏幕而不是像上次的伤害显示一样都在蓝图里拖，类似前面在伤害显示UMG里做过的那样，先拿到要在哪个Actor展示这个Actor的3D世界坐标位置转成2D的屏幕坐标，然后根据用户屏幕的DPI缩放Scale得到正确的显示2D坐标位置，最后把要展示的UI用`SetRenderTranslation `展示出来，这里用`ParentSizeBox`



需要向蓝图暴露一个 `AttachedActor `用来告诉UMG这是绑到谁身上显示的UI

加了个UI显示的偏移量，比如血量应当向上偏移在角色头顶

用 `UPROPERTY(meta = (BindWidget))` 来获取蓝图UMG中控件，这个控件的变量名必须和蓝图里的一致

如果绑定的Actor已销毁，则不渲染到屏幕

```cpp
UPROPERTY(BlueprintReadOnly, Category = "UI")
AActor* AttachedActor;

UPROPERTY(EditAnywhere, Category = "UI")
FVector WorldOffset;

UPROPERTY(meta = (BindWidget))
USizeBox* ParentSizeBox;

void USWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FVector2D ScreenPosition;

	if (!IsValid(AttachedActor))
	{
		RemoveFromParent();

		UE_LOG(LogTemp, Warning, TEXT("AttachedActor no longer vaild, removing HealthBar Widget"));
		return;
	}

	if (UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), AttachedActor->GetActorLocation() + WorldOffset, ScreenPosition))
	{
		float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
		ScreenPosition /= Scale;

		if (ParentSizeBox)
		{
			ParentSizeBox->SetRenderTranslation(ScreenPosition);
		}
	}
}
```



### 以`WorldUserWidget`为基类创建一个BP

BP里加一个`sizebox`控件，命名为上面的`ParentSizeBox`，对齐和之前一样用(0.5, 0.5)，设置`AutoSize`自适应下面图片大小

BP里加一个image控件，Material用之前做的`Health Bar`

事件绑定里在Construct的时候绑定`OnHealthChanged`



当受到伤害的时候更新Material里的`ProgressAlpha`来改变血量条，然后判断如果已经死了就销毁

![1697359256089](TyporaPic\1697359256089.png)



#### 第一次被命中的时候显示的血量条是错误的

因为第一次被命中的时候才开始创建这个UMG，然后在这个构造函数里才绑定上`OnHealthChanged`事件，所以第一次显示的是默认值，后面才是实时更新的



### 在`AICharacter`的`OnHealthChanged`里触发创建`HealthBar Widget` 和 `AddToViewport`

之前是在蓝图里拖的，这次改成在cpp里写

用一个变量来存储创建的Widget，这样可以用来防止创建多个重复UI



在`AddToViewport`前把`HealthBarWidget`要设置的成员变量设置好

```cpp
if (!ActiveHealthBar)
{
    ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
    if (ActiveHealthBar)
    {
        ActiveHealthBar->BindAttachedActor(this);
        ActiveHealthBar->AddToViewport();
    }
}
```



## 统一玩家的主屏幕的UMG到一个里

新建一个`Main_HUD `的user widget

然后把 `Player Health Widget `和` CorssHair Widget `都拖进来

`CorssHair Widget `需要对齐到中间



### 加个游戏运行时间

为了后面的多人游戏做准备，这里的时间用服务器运行时间，`GetGameState`->`GetServerWorldTimeSeconds`



### 加个得分

每击杀一个Minion都可以增加得分，现在先只做UI展示，逻辑在后面的Assignment做



### 做ui的技巧

做每个分ui的时候先用`canvas panel`包着拖ui，做好了要往`main_hud`里挪的时候再把`canvas panel`给replace to child 去除掉

ui编辑器的右上角可以调整填充屏幕(主ui)、屏幕上自定义(在ui编辑器内随便拖不会影响真实效果)、屏幕上所需(分ui)等展示方式





## 设置玩家出生地

之前是直接拖`Character_BP`到关卡编辑器里然后为`默认Pawn`，现在改成用`GameModeBase` + `PlayerStart `设置

在` GameModeBase_BP `里可以设置` Default Pawn Class` 为`Character_BP`，然后在项目设置里设置 `Global Default Game Mode `为 `GameModeBase_BP `

`World Setting`里的`GameMode`设置会覆盖项目设置里的，即优先级更高





## 添加控制台指令

用`UFUNCTION(Exec) `描述函数可以把这个函数添加到控制台指令中，当这个函数在`Player Controller`、 `Playing Character`、`GameMode`、`CheatManager`等少数几个类内才会生效



### 加了个一键秒杀Bot的指令结果发现直接秒杀的话怪物血条不会消失

应该是在同一帧触发了UMG的创建以及`OnHealthChaned`判断已经死亡要去`RemoveFromParent`，但是这个UMG的对象引用这一帧还没有被释放所以删不掉



在蓝图里加个`Delay Until Next Tick `再 `RemoveFromParent`就好了

![1697365601023](TyporaPic\1697365601023.png)



