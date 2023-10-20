// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "SAttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SGameplayFunctionLibrary.h"
#include "SActionComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SActionEffect.h"

// Sets default values
ASMagicProjectile::ASMagicProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 子弹生存时间
	InitialLifeSpan = 3.0f;
	DamageAmount = 20.0f;
}

void ASMagicProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);
}

void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// 另一种方式拿到GameplayTag
		// static FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag("Status.Parrying");
		USActionComponent* ActionComp = USActionComponent::GetActions(OtherActor);
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MovementComp->Velocity *= -1;
			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}

		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
			if (ActionComp && HasAuthority())
			{
				ActionComp->AddAction(GetInstigator(), EffectClass);
			}
		}

	}
}
