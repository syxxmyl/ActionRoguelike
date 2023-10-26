# Assignment 4 (Expanding AI Behavior) 

## AI怎么发射不了子弹了

给AI加了个`DamagePopup`的UMG以后发现AI怎么不发射子弹了，调试发现Spawn的`MagicProjectile` 创建完返回的时候是个nullptr，关卡编辑器一看发现每次发射的时候AI身上都会弹新加的`DamagePopup`，怀疑是子弹创建的时候和自己重叠然后没把自己过滤，在RangedAttack的这个TaskNode里加上

```cpp
SpawnParams.Instigator = AICharacter;
```

以后就好了(



## 加个Task回血

拿到`AttributeComp` 然后设置Health就行



## 加个Service查询是否低血量

拿到`AttributeComp` 判断当前Health的比例，低血量的门槛值暴露给蓝图



## 加个EQS查询回血的位置

远离TargetActor + 靠近当前自身的位置



## 连行为树

设定回血的优先级要高于攻击，因此回血的放到最左边

一开始用Selector 连下面一套流程，结果之执行了EQS，发现后改成Sequence就好了

Heal Self Wait 先设置了5秒，类似Apex拉大电拉上了才能回这种，现在先简单用wait 5s 代替

![1697173278542](TyporaPic\1697173278542.png)

