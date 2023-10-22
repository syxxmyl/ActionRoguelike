// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGameplayInterface.h"
#include "SItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ASItemChest : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASItemChest();

	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

	void OnActorLoaded_Implementation() override;

protected:
	UFUNCTION()
	void OnRep_LidOpened();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LidMesh;

	UPROPERTY(EditAnywhere)
	float TargetPitch;

	UPROPERTY(ReplicatedUsing = "OnRep_LidOpened", BlueprintReadOnly, SaveGame) // RepNotify
	bool bLidOpened;

};
