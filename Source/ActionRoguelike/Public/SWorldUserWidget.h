// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWorldUserWidget.generated.h"


class USizeBox;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USWorldUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AActor* AttachedActor;

	UPROPERTY(EditAnywhere, Category = "UI")
	FVector WorldOffset;

	UPROPERTY(meta = (BindWidget))
	USizeBox* ParentSizeBox;


public:
	void BindAttachedActor(AActor* TargetActor);
};
