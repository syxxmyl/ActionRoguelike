# Lecture 29 – Wrapping Up



## 加一个Projectile的碰撞通道类型

项目设置里加一个新的`Collision Channel`，默认响应类型为`Overlap`

![1698226318546](TyporaPic\1698226318546.png)



修改之前预设Preset的`Projectile`类型的对象类型为新加的Projectile

![1698226362225](TyporaPic\1698226362225.png)



`Pawn`的对于Projectile的碰撞响应修改为忽略

`Block`和`BlockAll`仍然维持阻挡



### 修改AICharacter的碰撞类型

刚才在上面修改了CharacterMesh类型对Projectile为默认的`Overlap`了，所以之前HitResult命中AI的是胶囊体`Capsule`而不是`Mesh `为了让命中能产生效果而设置的内容可以注释掉了

```cpp
// GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
```



### 修改SCharacter的Mesh碰撞响应事件

这样才能触发被击中的事件

```cpp
GetMesh()->SetGenerateOverlapEvents(true);
```



## 修改对相机的碰撞Channel，这样在观察靠近在一起的角色的时候摄像机不会被挤

Pawn和CharacterMesh对`Camera`的碰撞类型改为忽略



## 可以保存加载多个存档

现在是在GameModeBase里写死的`SaveGame01`

改成从MainMenu的蓝图里执行`OpenLevel`传入的`Options`里解析具体要哪个存档

```cpp
void ASGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FString SelectedSaveSlot = UGameplayStatics::ParseOption(Options, "SaveGame");
	if (SelectedSaveSlot.Len() > 0)
	{
		SlotName = SelectedSaveSlot;
	}

	LoadSaveGame();
}
```



修改`MainMenuWidget `，在`Click HostGame`的那个Button的时候`OpenLevel`的`Options`改成拼接字符串，加了个输入框用来接受存档名输入

![1698228356652](TyporaPic\1698228356652.png)





## 一些Widget目标不在屏幕里会默认绘制到屏幕的左上角(0,0)处

### DamagePopUpWidget

`Project World To Screen `有个return的`bool`类型的值，如果return的是false就代表不在屏幕里失败了，这时我们可以把`damageText`这个组件的可见性设为 折叠

不设置整个` DamagePopUpWidget `的可见性是因为如果在蓝图中设置整个Widget的可见性为折叠，那就不会再运行Tick了

![1698230814825](TyporaPic\1698230814825.png)



### 继承自自己写的SWorldUserWidget的

和蓝图处理类似，用`ProjectWorldToScreen `的返回值设置可见性，和蓝图的区别是这里是设置整个组件的，在蓝图里只设置了`DamageText`子组件

```cpp
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

	bool bIsOnScreen = UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), AttachedActor->GetActorLocation() + WorldOffset, ScreenPosition);

	if (bIsOnScreen)
	{
		float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
		ScreenPosition /= Scale;

		if (ParentSizeBox)
		{
			ParentSizeBox->SetRenderTranslation(ScreenPosition);
		}
	}
	
	if (ParentSizeBox)
	{
		ParentSizeBox->SetVisibility(bIsOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
```



