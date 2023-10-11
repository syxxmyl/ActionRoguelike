// Fill out your copyright notice in the Description page of Project Settings.


#include "STargetDummy.h"
#include "Components/StaticMeshComponent.h"
#include "SAttributeComponent.h"

// Sets default values
ASTargetDummy::ASTargetDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	RootComponent = MeshComp;

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASTargetDummy::OnHealthChanged);

	HitFlashParamName = "TimeToHit";
}

// Called when the game starts or when spawned
void ASTargetDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASTargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASTargetDummy::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	// 代表受到了伤害，展现一种材质，以后受到治疗可以展现其他样式的材质
	if (Delta < 0.0f)
	{
		MeshComp->SetScalarParameterValueOnMaterials(HitFlashParamName, GetWorld()->GetTimeSeconds());
	}
}

