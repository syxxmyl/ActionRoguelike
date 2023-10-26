# Lecture 5 - Blueprints 101

Blueprint在ue3的时候叫Kismet，引擎代码有些地方还在沿用



## 官方学习实例

https://docs.unrealengine.com/5.1/zh-CN/content-examples-sample-project-for-unreal-engine/



## 拉下拉杆开宝箱

可以仿照开宝箱写cpp代码，也可以用蓝图拖出来

蓝图拖的话需要在蓝图里设置一个新的Actor类型的变量用来保存选中的Actor，并设置它是可编辑的，然后要为他指定一个actor，简单的测试的话可以直接去关卡编辑器里设置，或者用前面的射线检测来set具体的actor

![1696822625309](TyporaPic\1696822625309.png)



## 拉下拉杆让油桶爆炸

首先在油桶的蓝图中新增自定义事件，用于拉杆触发

![1696824076074](TyporaPic\1696824076074.png)

修改拉杆蓝图里的执行流，将选中的Actor转换为油桶类，转换成功的话执行Explode事件

![1696824172036](TyporaPic\1696824172036.png)



## 在蓝图中重写cpp实现过的宝箱的Interact函数

cpp里是直接设置了宝箱盖的rotation达到下一帧就开启完毕，现在要改成蓝图动画缓慢旋开，由于实现的时候Interact接口是`BlueprintNativeEvent` 的，所以蓝图的修改会覆盖掉cpp的实现

如果想保留cpp里写好的逻辑，可以在蓝图里右键该函数->`Add call to parent function` 并连线，类似`Super::xxx`



### 制作一个简单的开宝箱宝箱盖转动的动画

`Add Timeline` 节点 新建一个`float line` 命名为`pitch`，然后在里面 新建两个关键帧节点(0,0) (0.5,110)，用来旋转宝箱顶，可以调整节点长度为1秒+设置使用最后一个关键帧，这样动画播放到最后一个(0.5,110) 就会结束了

![1696824835307](TyporaPic\1696824835307.png)



#### 拼蓝图

![1696825510109](TyporaPic\1696825510109.png)





#### 直观看到蓝图执行流

选择要调试的对象，比如这个宝箱，然后打开宝箱就可以看到蓝图执行流了

![1696825449649](TyporaPic\1696825449649.png)



右击Pitch选择监视该值还可以看到变化过程

![1696825631352](TyporaPic\1696825631352.png)



### 加一个开宝箱的粒子效果

把粒子效果的`Auto Active`关掉，改成当`AnimateLid Finished`的时候才触发粒子的Active



### 能开关宝箱

用`Flip Flop` 蓝图组件 连接到Reverse节点，就可以反向执行动画把他关上了

![1696826436820](TyporaPic\1696826436820.png)



## 优化子弹内容

### 子弹命中时销毁

蓝图里给负责碰撞处理的Sphere组件加一个`On Component Hit` 的事件一路连到`DestoryActor`即可， 中间可以加`Spawn Emiter At Location` 添加一个粒子效果之类的



### 给子弹设置Instigator

常用于击杀得分应该给谁这种情况

在Character创建该子弹的时候有个参数列表，在那里加上，然后就可以在蓝图里Get到了

```
FActorSpawnParameters SpawnParams;
SpawnParams.Instigator = this;
```



### 蓝图连一下为了不和自己的手部碰撞

![1696828233709](TyporaPic\1696828233709.png)



### 为什么修改后子弹还是会和自己碰撞停在原地

因为移动是由`MovementComponent`负责的，此时已经和自己碰撞完毕立刻停止了，所以上面的方法不行



### 更好的解决方案

在子弹`BeginPlay`的时候设置`Ignore Actor When Moving`

注意要把子弹的Sphere组件暴露给蓝图

```cpp
// 处理碰撞
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
USphereComponent* SphereComp;
```



![1696828454972](TyporaPic\1696828454972.png)



### 用蓝图快速创建一个定时生成子弹的类

用蓝图可以很方便的拖出来一个这种功能的类，可以定时生成子弹，供快速调试比如子弹击中目标造成伤害之类的情况

![1696829153452](TyporaPic\1696829153452.png)



