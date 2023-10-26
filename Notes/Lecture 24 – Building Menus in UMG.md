# Lecture 24 – Building Menus in UMG

## 制作一个主菜单界面UMG

### 制作一个主菜单按钮 `MenuButton `的UMG供复用

不需要 `Canvas Panel `，直接拖一个 Button ，再拖一个 Text 到 Button 里

#### 允许修改按钮的文本Text

把Text设置为变量并重命名为 `ButtonText `

在图标里新建一个Text 文本类型的变量 Text 并设置可编辑实例

拖拽到蓝图里后把它设置为非本地化，这样他作为一个基类蓝图是非本地化的，后续需要本地化的内容再单独打开那个实例的本地化

在`Event Pre Construct `里设置`ButtonText`的文本，这样当在其他界面用这个按钮的时候就可以修改文本的内容了

![1698046306970](TyporaPic\1698046306970.png)



#### 允许修改按钮的点击事件

在设计器里选择按钮，然后在右边的细节里找到点击时的事件并单击，在事件图表的左边有个事件分发器，新建一个委托事件命名为`OnClicked`，就和之前在cpp里用宏定义写`OnHealthChanged`差不多

把他拖到蓝图里连在 点击时的Event后

![1698046686561](TyporaPic\1698046686561.png)



#### 美化一下显示

给`ButtonText`设置填充间距和字体

给Button设置` Normal`、`Hovered`、`Pressed `三种不同情况下的Image



### 制作MainMenu

#### 拖控件修改布局

在Canvas Panel里拖拽一个垂直框，里面放三个上面做的 `MenuButton `依次用来处理`Host`、`Join`、`Quit` 事件

垂直框设置锚点居中，偏移（0,0），对齐（0.5,0.5），这样三个按钮在屏幕的中心

`Canvas Panel`的右下角有个缩放标，可以拖动来看不同分辨率下UI的表现效果

三个按钮各自垂直对齐各自的位置向上、居中、向下，只有中间的尺寸设置为填充，其他两个都是自动

![1698047834817](TyporaPic\1698047834817.png)



#### 实现 Quit Game 点击事件

直接连上Quit Game事件即可

![1698047929501](TyporaPic\1698047929501.png)



#### 实现Host Game点击事件

直接连上Open Level事件，然后填上关卡名，在Options里填上`?listen` 表示作为监听服务器开始

![1698048059698](TyporaPic\1698048059698.png)



#### 实现 Join Game 点击事件

目前先用ip加入，因此UI上需要一个Text Box来输入ip 地址，为了不破坏之前调整好的界面布局，直接右击`Join Game Button `选择包裹->垂直框，这样新的垂直框就代替了加入游戏按钮之前在整体布局里的位置设置

然后在加入游戏按钮所在的垂直框里添加一个`Text Box`，给他设置字体、提示文本

![1698048702444](TyporaPic\1698048702444.png)



![1698048709217](TyporaPic\1698048709217.png)



然后设置点击事件，和`Host Game Button`的类似，也是直接连到`OpenLevel`，不过`LevelName`用的是IP地址

要从TextBox的文本`GetText `->` ToString` ->`StringToName `才能把输入的ip转换为可处理内容

![1698048757459](TyporaPic\1698048757459.png)



### 添加MainMenu到关卡里

#### 创建一个新的Level用来显示MainMenu

任何一个UI都需要在一个level里

创建一个新的关卡`MainMenu_Entry`

然后在项目设置->地图和模式->默认地图->默认游戏地图换成刚才创建的新Level



#### 创建一个新的GameMode蓝图类用来处理MainMenu相关的逻辑内容

直接继承自`GameModeBase`，然后在细节里设置默认Pawn类为 None 

在蓝图里Begin Play 的时候创建`MainMenu_Widget` ，并`Add To Viewport`

![1698049371014](TyporaPic\1698049371014.png)



在`MainMenu_Entry `这个关卡的World Setting里重载这个新的GameMode



#### 设置合适的位置展示MainMenu

在关卡里拖一个Camera进去，可以右击选择控制摄像机，直接在关卡内移动选择一个好的位置，选择好以后点击左上角即可回到关卡编辑里

然后回到这个摄像机的细节设置里，设置` Auto Activate for Player` 为 player0， 这样加入进来的玩家就是启用这个摄像机了



### 设置鼠标控制

需要在新创建的`MainMenuGameMode`里设置一些内容，如`bShowMouseCursor`、`Set Input Mode UI  Only`

因为是主菜单的Level，所以可以用`Get Player Controller  0 `的形式来拿到玩家

![1698050237932](TyporaPic\1698050237932.png)



#### 在进入关卡后把设置的输入模式改回来，否则不能正常游戏了（

在`BP_PlayerController` 里的`BeginPlayerState `事件`Add Main_HUD To Viewport` 后 设置输入模式为`Game Only`

![1698050449388](TyporaPic\1698050449388.png)



### 测试多人游戏的情况

选择玩家数为2个，网络模式为`Standalone`，然后就可以一个当Host一个Join连进去了





## 制作一个游戏中暂停的菜单界面UMG



### 制作PauseMenuWidget

#### 拼界面

和之前做的`MainMenu`类似，拖两个`MenuButton`到`Canvas Panel`里，然后把他们用垂直框包起来，这次可以在他俩中间加一个`Spacer `控件，就可以提供间隔了

记得把这两个按钮的本地化打开，因为按钮的基类本地化是关掉的

![1698051713127](TyporaPic\1698051713127.png)



![1698051719669](TyporaPic\1698051719669.png)



#### 实现Return To MainMenu 点击事件

直接`OpenLevel `打开`MainMenu_Entry `主界面关卡即可

![1698051891675](TyporaPic\1698051891675.png)



### 用cpp实现打开界面的逻辑

在`PlayerController`里绑定打开界面的按键，处理逻辑

每次按键触发事件的时候判断是否已实例化，已实例化就是关闭UI，没实例化就是打开UI

和上面MainMenu对玩家输入的修改一致，打开UI时显示鼠标、输入模式切换为`UI Only`，关闭UI时隐藏鼠标、切换回`Game Only`



```cpp
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<UUserWidget> PauseMenuClass;

UPROPERTY()
UUserWidget* PauseMenuInstance;

UFUNCTION(BlueprintCallable)
void TogglePauseMenu();

void SetupInputComponent() override;
	
	
	

void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("PauseMenu", IE_Pressed, this, &ASPlayerController::TogglePauseMenu);
}
	
	
void ASPlayerController::TogglePauseMenu()
{
    if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
    {
        PauseMenuInstance->RemoveFromParent();
        PauseMenuInstance = nullptr;

        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());

        return;
    }

    PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
    if (PauseMenuInstance)
    {
        PauseMenuInstance->AddToViewport(100);
        bShowMouseCursor = true;

        SetInputMode(FInputModeUIOnly());
    }
}
```



### 用蓝图实现Return To Game 的点击事件

`Get Owning Player` ->` Cast to SPlayerController` -> `TogglePauseMenu`

![1698053368212](TyporaPic\1698053368212.png)



