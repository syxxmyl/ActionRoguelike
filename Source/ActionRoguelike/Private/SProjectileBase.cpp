// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ASProjectileBase::ASProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");
	RootComponent = SphereComp;

	APawn* MyOwner = GetInstigator();
	SphereComp->IgnoreActorWhenMoving(MyOwner, true);

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 1000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bInitialVelocityInLocalSpace = true;
	MovementComp->ProjectileGravityScale = 0.0f;

	AudioComp = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	AudioComp->SetupAttachment(RootComponent);

	ImpactSoundComp = CreateDefaultSubobject<USoundCue>("ImpactSoundComp");

	InnerRadius = 200.0f;
	OuterRadius = 2000.0f;
}

// Called when the game starts or when spawned
void ASProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	AudioComp->Activate();
}

void ASProjectileBase::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

void ASProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComp->OnComponentHit.AddDynamic(this, &ASProjectileBase::OnActorHit);
}

void ASProjectileBase::Explode_Implementation()
{
	// 确保只生成一次，因为生成一次就会走Destory()标记为删除了
	if (IsValid(this))
	{
		if (ImpactVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());
		}

		if (ImpactSoundComp)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSoundComp, GetActorLocation());
		}

		if (ensure(ImpactCameraShake))
		{
			UGameplayStatics::PlayWorldCameraShake(this, ImpactCameraShake, GetActorLocation(), InnerRadius, OuterRadius);
		}

		EffectComp->DeactivateSystem();
		MovementComp->StopMovementImmediately();
		SetActorEnableCollision(false);

		// 这一帧结束才真正删除
		Destroy();
	}
}
