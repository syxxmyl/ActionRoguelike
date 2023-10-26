# Lecture 2 - Project Start & Version Control

## UE类的继承层级

```cpp
Object					所有子类的父类
  ->
    Actor				可以被放置或生成在世界里的物体类
      ->
        Pawn			可以被玩家或AI控制的类，接受输入
          ->
            Character	实现了移动(walking movement)功能的Pawn子类
```



## UE类的引擎给的前缀

```cpp
U		继承自UObject的子类
A		继承自Actor的子类
F		Struct
E		Enum
I		Interfaces
```



## 胶囊体组件 `CapsuleComponent` 

https://docs.unrealengine.com/4.26/zh-CN/Basics/Components/Shapes/

处理碰撞和重叠事件



## 箭头组件 `ArrowComponent`

https://docs.unrealengine.com/4.26/zh-CN/Basics/Components/Shapes/

 由直线和箭头组成的形状，表示对象的朝向 



## 弹簧臂组件` SpringArmComponent ` 和相机组件 ` CameraComponent `

https://docs.unrealengine.com/4.27/zh-CN/Basics/Components/Camera/

弹簧臂组件主要用于 使它的子对象与父对象之间保持固定距离；但是如果发生遮挡，将缩短这段距离。遮挡消失后，距离又会恢复正常。 

 摄像机组件用于 为 Actor 绑定一个摄像机视角子对象。 



## Unreal Reflection System

https://docs.unrealengine.com/5.1/zh-CN/reflection-system-in-unreal-engine/

`UPROPERTY`	`UFUNCTION`	`USTRUCT` ……

主要用于 蓝图读写控制、网络行为、内存管理



## 直接用github里content蓝图动画资源会提示已损坏无法使用

github里面的资源是已完成版本的，存在各种嵌套引用，失效的蓝图全部都继承自 自定义C++类，项目迁移并不会把C++类也给导过去 而导致蓝图损坏 

git clone 了整个工程然后回退到提交这节课的版本，再迁移这个时候的资源就成功了