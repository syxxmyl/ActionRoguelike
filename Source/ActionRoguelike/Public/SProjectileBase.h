// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SProjectileBase.generated.h"


class UParticleSystem;
class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UAudioComponent;
class USoundCue;
class UCameraShakeBase;

UCLASS(ABSTRACT)
class ACTIONROGUELIKE_API ASProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASProjectileBase();

protected:
	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// BlueprintNativeEvent			C++和蓝图都可以编写逻辑，但是蓝图的优先级更高
	// BlueprintCallable			C++编写逻辑，蓝图直接调用
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Explode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ImpactVFX;

	// 处理碰撞
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USphereComponent* SphereComp;

	// 处理运动
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* MovementComp;

	// 处理粒子效果
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* EffectComp;

	// 处理音效
	UPROPERTY(EditAnywhere, Category = "Components")
	UAudioComponent* AudioComp;

	// 处理命中音效
	UPROPERTY(EditAnywhere, Category = "Components")
	USoundCue* ImpactSoundComp;

	UPROPERTY(EditAnywhere, Category = "HitCameraShake")
	TSubclassOf<UCameraShakeBase> ImpactCameraShake;

	UPROPERTY(EditAnywhere, Category = "HitCameraShake")
	float InnerRadius;

	UPROPERTY(EditAnywhere, Category = "HitCameraShake")
	float OuterRadius;
};
