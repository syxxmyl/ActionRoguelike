// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class ASProjectileBase;
class USInteractionComponent;
class UAnimMontage;
class USAttributeComponent;

UCLASS()
class ACTIONROGUELIKE_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComp;


	UPROPERTY(VisibleAnywhere, Category = "PrimaryInteract")
	USInteractionComponent* InteractionComp;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* PrimaryAttackAnim;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float ProjectileSpawnDelayTime;

	FTimerHandle TimerHandle_PrimaryAttack;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<ASProjectileBase> ClassToSpawn;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<ASProjectileBase> PrimaryAttackProjectile;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<ASProjectileBase> BlackholeProjectile;
	
	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<ASProjectileBase> DashProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USAttributeComponent* AttributeComp;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName HandLocationSocketName;

	UPROPERTY(EditAnywhere, Category = "Hit")
	FName HitFlashParamName;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);

	void MoveRight(float value);

	void PrimaryAttack();

	void PrimaryAttack_TimeElapsed();

	void PrimaryInteract();

	void SwitchMagicProjectile();

	void SwitchBlackholeProjectile();

	void SwitchDashProjectile();

	FRotator CalcProjectileSpawnRotation(FVector HandLocation);

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);

	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
