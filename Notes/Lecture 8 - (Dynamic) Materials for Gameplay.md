# Lecture 8 - (Dynamic) Materials for Gameplay

## 创建一个材质

### 更方便的调整材质参数

创建的材质如果要修改效果必须要重新保存编译才能看到效果，因此可以把需要调整的数值右键转换为参数，然后在Content里右击该材质选择创建该材质的实例，用这个实例就可以边编辑边在关卡编辑器里看效果了



### 实时显示材质的相关节点变化

实时更新->所有节点预览，就可以看到点开了右上角角标的预览图了

![1696938149425](TyporaPic\1696938149425.png)



或者创建一个`DebugScalarValues` 函数节点

![1696938356511](TyporaPic\1696938356511.png)



或者右键某个节点->开始预览节点，则会编译到这个节点为止的之前的所有节点然后显示，适合debug某一分支的情况

![1696938512623](TyporaPic\1696938512623.png)



## 在蓝图中修改材质的参数

比如 `StaticMesh`加了一个有参数的Material，然后就可以在蓝图里把这个Mesh拖进来，然后连接`Set Scalar Paramter Value On Materails`  然后修改具体的参数内容了



##### 但是最好还是直接用Material里的那些函数节点而不是蓝图，因为Material里的那些节点会被编译为HLSL，发往GPU处理，不会占用CPU



##### 比较好的做法是类似上面的UMG事件驱动UI变化，当有事件发生的时候才用`Set Scalar Paramter Value On Materails` 变化Material的Parameter



## 在cpp中修改材质的参数

有一个很巧妙的trick，在材质里设置一个参数为传入的`TimeToHit`，在Material里他会和直接就能在Material里拿到的`GameTimeSecond`做减法运算，用差值来表现出"闪光"

![1696944021579](TyporaPic\1696944021579.png)

这样我们就可以仿照蓝图修改材质参数的方法，在cpp里绑定事件，当事件触发的时候用

```cpp
MeshComp->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->GetTimeSeconds());
```

修改材质的参数值，以此达到事件触发闪光 这种效果



绑定的事件就是上面写的当`MagicProjectile`和Actor产生Overlap的时候触发的

```cpp
UPROPERTY(BlueprintAssignable)
FOnHealthChanged OnHealthChanged;


AttributeComp->OnHealthChanged.AddDynamic(this, &ASTargetDummy::OnHealthChanged);
```



### 更方便快速的调整效果

把要调整的内容提升为参数，然后把这个Material实例化出来，然后用这个实例化的Material在游戏中实时改变参数的值来快速调整效果



## 创建一个溶解材质

原理是用`BLEND_Masked`类型的材质时，如果像素值小于不透明蒙版剪切值`Opacity Mask Clip Value`，就不会被渲染出来，因此可以通过一个渐变的数值更改让像素值逐渐的从大于临界值到小于临界值，以此实现从可见->不可见的溶解过程



## 创建一个材质函数`Material Function`

在Content里右键材质-材质函数，然后可以用上面做过的闪光的相关内容拷贝进来然后把闪光材质里的内容替换成这个材质函数

![1696946175511](TyporaPic\1696946175511.png)

### 把材质函数公开到材质函数库中

左边栏有一个公开到库默认是不勾选的，勾选即可



### 给函数指定一个输入

右击选择FunctionInput，然后就可以从外部传入值了，也可以给他指定将预览值作为默认值，这样有些不传入参的也可以正常使用



## 创建一个UI用的材质

`MaterialDomain` 改为UI，借助`LinearGradient` 的 `UGradient`，我们可以得到一个x方向的从0-1的渐变值，借助if和参数`ProgressAlpha`，我们可以通过改变参数的值来改变整个进度条哪一部分是红色的

![1696946884207](TyporaPic\1696946884207.png)



在Health的UMG里用一个新的image控件替换掉原本的`progressbar`控件

然后修改UMG的蓝图，原本是直接用新的百分比值设置`progressbar`，现在改成用新的百分比值设置`HealthBar Material` 的`ProgressAlpha`参数，以此来改变这个材质的样式，从而看起来是血条发生了变化



### 整个的流程是

`MagicProjectile` 和玩家重叠触发了子弹的 `ActorOverlap` 事件

->修改了玩家的 `AttributeComp` 的血量成员变量，触发`AttributeComp` 的 `OnHealthChanged`事件

->然后通知监听了这个`AttributeComp`的`OnHealthChanged`事件的UMG蓝图，这个蓝图去拿到当前玩家的血量和最大血量做除法算出现在还剩百分之多少的血量

->用这个值更新UMG的image控件的Material的`ProgressAlpha`参数，从而改变这个材质的样式