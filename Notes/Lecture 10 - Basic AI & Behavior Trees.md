# Lecture 10 - Basic AI & Behavior Trees

## UE 的 AI 系统

Navigation Mesh	为寻路提供数据

Behavior Tree		  AI的行为选择

Blackboard			 存储AI相关的数据

Environment Query System	检测World中的其他Objects

PawnSensing & AIPerception	对其他事物的感官

### Behavior Tree

##### Sequence 和 Selector

Sequence 会从左到右依次执行所有子结点的内容，直到其中一个子结点返回Failure

Selector 会从左到右依次检查 每个子结点的 Decorator 选择其中一个允许执行的执行

##### Task Service Decorator

Task	具体要执行的行为

Service	一直重复执行，类似后台任务

Decorator 	条件检查，流量控制



## cpp工程添加AIModule模块

ue编辑器帮忙在`.uproject` 里添加好了 `"AIModule"` 模块，否则编译会报错，保险起见在Build.cs里加上`AIModule`



## AI的Controller用的行为树的变量值由cpp控制的大致流程

在Blackboard里指定变量类型，将这个Blackboard指定给Behavior Tree，然后把这个Behavior Tree指定给AI使用的Controller，去对应的Controller里`GetBlackboardComponent()->SetValueXXX` 赋上值，最后把这个Controller指定给AI的Character



## 创建一个行为树使用的Service

继承自BTService，去BTService头文件查找发现要继承TickNode

```cpp
void USBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // 拿到行为树需要的数据存储在的BlackboardComp
	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if (ensure(BlackBoardComp))
	{
        // 从Blackboard 里拿到玩家
		AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject("TargetActor"));
		if (TargetActor)
		{
            // 通过拿到AIController 拿到AIPawn
			AAIController* AIController = OwnerComp.GetAIOwner();
			if (ensure(AIController))
			{
				APawn* AIPawn = AIController->GetPawn();
				if (ensure(AIPawn))
				{
                    // 比较AIPawn 和 玩家 之间的距离 以及AI是否可见玩家
					float Distance = FVector::Distance(TargetActor->GetActorLocation(), AIPawn->GetActorLocation());
					bool bWithinRange = Distance < 2000.0f;
					bool bHasLostOfSight = false;
					if (bWithinRange)
					{
						bHasLostOfSight = AIController->LineOfSightTo(TargetActor);
					}

                    // 根据结果set Blackboard里的变量值
					BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, bWithinRange && bHasLostOfSight);
				}
			}
		}
	}
}
```

用`FBlackboardKeySelector.SelectedKeyName`

```cpp
UPROPERTY(EditAnywhere, Category = "AI")
FBlackboardKeySelector AttackRangeKey;
```

代替字符串硬编码的用于Set的变量名

##### 编译会报错链接失败，要去Build.cs里加上`GameplayTasks`



## 拖一个简单的行为树

https://zhuanlan.zhihu.com/p/368889019

 遍历顺序简单来说，就是遍历根节点的每一个service，然后遍历每一个子节点的decorator，然后再是每一个子节点task，然后继续递归这些子节点。 

```text
父节点（Composite节点）本身
父节点（Composite节点）的所有service -> 
子节点（非Composite节点）的decorator ->
子节点（非Composite节点）的service ->
子节点（非Composite节点）本身
```



首先是运行上面自己写的`CheckAttackRange`的`BT_Service`会定时执行`TickNode` 函数，更新Service里绑定的`AttackRangeKey`对应的Blackboard里的`WithinAttackRange`的值

然后先走左子结点，判断Blackboard里的`WithinAttackRange`是否为False，如果为False 则满足Decorator则代表AI离玩家过远或看不到玩家，然后执行子结点Move To Player

如果左子结点的`WithinAttackRange` 为True，则不满足Decorator的条件，转而选择右子结点的Wait

通常情况下就需要等Wait执行完才会走下一次行为树遍历了，也就是AI会站在原地等5秒再一次判断距离是否需要移动，如果想变的及时，就可以给`Outside Attack Range`  这个Decorator加一个 AbortMode = Lower Priority(Both也会因为判断当前执行的Wait不属于Decorator这一条子树而转为Lower Priority)，这代表当行为树处于Wait的Task执行过程中，Service修改了`WithinAttackRange`的值时会通知打断当前执行的Wait的Task，重新搜索执行的Task也就是Selector的左子树

![1697102761435](TyporaPic\1697102761435.png)

