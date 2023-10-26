# Assignment 3 (Health - Damage, Audio, Projectiles, UI)

## 给`MagicProjectile`加个声音组件

放到`ProjectileBase`里了

```cpp
// 处理音效
UPROPERTY(EditAnywhere, Category = "Components")
UAudioComponent* AudioComp;

// 处理命中音效
UPROPERTY(EditAnywhere, Category = "Components")
USoundCue* ImpactSoundComp;
```



`AudioComp`要在整个子弹的生命周期都发出声音，所以在`BeginPlay`的时候激活即可

```cpp
AudioComp->Activate();
```



`ImpactSoundComp`放在`OnHit`的函数里，只要命中的时候播放一次就行

```cpp
if (ImpactSoundComp)
{
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSoundComp, GetActorLocation());
}
```



## 玩家Mesh加个受击闪光的材质

材质Mesh的自发光颜色用前面自己写的 `HitFlash`材质函数连上，Character的`OnHealthChanged`里判断下Delta小于0代表是受到伤害了，就set一下材质里的Parameter就行了

```cpp
void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(HitFlashParamName, GetWorld()->GetTimeSeconds());
	}
}
```



## `AttributeComp`加个`HealthMax` 属性

照着Health加一个就行，暴露给蓝图用



## 给受击弹窗的UMG加个伤害值显示和对应的Animate

当这个UMG被构造出来的时候代表受到攻击了，加个伤害值的变量，照着`AttachTo`的变量一起传进来，给Text绑定一个函数返回伤害值变量值

在UMG下面的动画拖个简单的字体变红再恢复的动画， 然后在UMG的`Event Construct` 蓝图拖一个`Play Animate`，后面跟一个`Delay` ->`Remove from parent`，Delay的时间和Animate的时间一样就行



## 加个回复药水交互回复血量

基类处理交互cd、碰撞检测的内容，派生类重写`Interact_Implementation `处理回复血量逻辑



## 发射子弹的时候加个flash特效，子弹`OnHit`的时候加个世界摄像机摇晃

一开始特效只指定了Mesh没指定骨骼socket，结果生成在了脚下

```cpp
UPROPERTY(EditAnywhere, Category = "Attack")
UParticleSystem* AttachedEffect;

if (AttachedEffect)
{
    UGameplayStatics::SpawnEmitterAttached(AttachedEffect, GetMesh(), HandLocationSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
}
```



半径要设大一点，不然感知不到

```cpp
UPROPERTY(EditAnywhere, Category = "HitCameraShake")
TSubclassOf<UCameraShakeBase> ImpactCameraShake;

UPROPERTY(EditAnywhere, Category = "HitCameraShake")
float InnerRadius;

UPROPERTY(EditAnywhere, Category = "HitCameraShake")
float OuterRadius;

if (ensure(ImpactCameraShake))
{
    UGameplayStatics::PlayWorldCameraShake(this, ImpactCameraShake, GetActorLocation(), InnerRadius, OuterRadius);
}
```



