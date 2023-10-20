// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_ProjectileAttack.h"
#include "Particles/ParticleSystem.h"
#include "SProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

USAction_ProjectileAttack::USAction_ProjectileAttack()
{
	HandLocationSocketName = "Muzzle_01";
	ProjectileSpawnDelayTime = 0.2f;
}

void USAction_ProjectileAttack::StartAction_Implementation(AActor* InstigatorActor)
{
	Super::StartAction_Implementation(InstigatorActor);

	ACharacter* Character = Cast<ACharacter>(InstigatorActor);
	if (Character)
	{
		Character->PlayAnimMontage(PrimaryAttackAnim);

		if (AttachedEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(AttachedEffect, Character->GetMesh(), HandLocationSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
		
			if (Character->HasAuthority())
			{
				FTimerHandle TimerHandle_AttackDelay;
				FTimerDelegate Delegate;
				Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

				GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, ProjectileSpawnDelayTime, false);
			}
		}
	}
}

FRotator FindLookAtRotation(FVector const& X)
{
	// https://zhuanlan.zhihu.com/p/108474984
	FVector const NewX = X.GetSafeNormal();
	FVector const UpVector = (FMath::Abs(NewX.Z) < (1.f - KINDA_SMALL_NUMBER)) ? FVector(0, 0, 1.f) : FVector(1.f, 0, 0);//得到原坐标轴的Z轴方向
	const FVector NewY = (UpVector ^ NewX).GetSafeNormal();//叉乘可得到Y'
	const FVector NewZ = NewX ^ NewY;//再次将X'与Y'叉乘即可得到Z'

	return FMatrix(NewX, NewY, NewZ, FVector::ZeroVector).Rotator();
}

FRotator CalcProjectileSpawnRotation(ACharacter* InstigatorCharacter, FVector HandLocation)
{
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	// 防止贴墙/抬头摄像机贴着地面 的时候碰撞检测到墙/地面 
	
	FVector TraceBeginLocation = InstigatorCharacter->GetPawnViewLocation() + InstigatorCharacter->GetControlRotation().Vector() * 20;
	FVector TraceEndLocation = TraceBeginLocation + InstigatorCharacter->GetControlRotation().Vector() * 5000;
	// DrawDebugLine(GetWorld(), TraceBeginLocation, TraceEndLocation, FColor::Green, false, 10.0f, 0, 2.0f);


	FCollisionShape Shape;
	Shape.SetSphere(20.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(InstigatorCharacter);

	FRotator SpawnRotation;

	FHitResult Hit;
	bool bBlockingHit = InstigatorCharacter->GetWorld()->SweepSingleByObjectType(Hit, TraceBeginLocation, TraceEndLocation, FQuat::Identity, ObjectQueryParams, Shape, Params);
	if (bBlockingHit)
	{
		TraceEndLocation = Hit.ImpactPoint;
	}

	// SpawnRotation =  FRotationMatrix::MakeFromX(TraceEndLocation - HandLocation).Rotator();
	SpawnRotation = FindLookAtRotation(TraceEndLocation - HandLocation);
	DrawDebugLine(InstigatorCharacter->GetWorld(), HandLocation, TraceEndLocation, FColor::Red, false, 10.0f, 0, 2.0f);
	return SpawnRotation;
}

void USAction_ProjectileAttack::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	FVector HandLocation = InstigatorCharacter->GetMesh()->GetSocketLocation(HandLocationSocketName);
	FTransform SpwanTM = FTransform(CalcProjectileSpawnRotation(InstigatorCharacter, HandLocation), HandLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = InstigatorCharacter;

	if (ensure(ClassToSpawn))
	{
		GetWorld()->SpawnActor<ASProjectileBase>(ClassToSpawn, SpwanTM, SpawnParams);
	}

	StopAction(InstigatorCharacter);
}
