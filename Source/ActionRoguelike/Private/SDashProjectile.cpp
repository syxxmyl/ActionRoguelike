// Fill out your copyright notice in the Description page of Project Settings.


#include "SDashProjectile.h"
#include "Components/SphereComponent.h"


// Sets default values
ASDashProjectile::ASDashProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProjectileDestoryDelayTime = 0.2f;

	SphereComp->SetCollisionObjectType(ECC_WorldStatic);
}

// Called when the game starts or when spawned
void ASDashProjectile::BeginPlay()
{
	Super::BeginPlay();

	APawn* MyOwner = GetInstigator();
	SphereComp->IgnoreActorWhenMoving(MyOwner, true);
	
	GetWorldTimerManager().SetTimer(TimerHandle_NormalDestory, this, &ASDashProjectile::NormalDestory, ProjectileDestoryDelayTime);
}

// Called every frame
void ASDashProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASDashProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComp->OnComponentHit.AddDynamic(this, &ASDashProjectile::OnActorHit);
}


void ASDashProjectile::NormalDestory()
{
	APawn* MyOwner = GetInstigator();
	if (ensure(MyOwner))
	{
		FVector DashLocation = GetActorLocation();
		MyOwner->SetActorLocation(DashLocation);
	}

	Destroy();
}

void ASDashProjectile::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	NormalDestory();
}
