// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SProjectileBase.h"
#include "SInteractionComponent.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;

	ProjectileSpawnDelayTime = 0.2f;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimaryAttack);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAction("SwitchMagicProjectile", IE_Pressed, this, &ASCharacter::SwitchMagicProjectile);
	PlayerInputComponent->BindAction("SwitchBlackholeProjectile", IE_Pressed, this, &ASCharacter::SwitchBlackholeProjectile);
}

void ASCharacter::MoveForward(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	AddMovementInput(ControlRot.Vector(), value);
}

void ASCharacter::MoveRight(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	FVector RightVec = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);

	AddMovementInput(RightVec, value);
}

void ASCharacter::PrimaryAttack()
{
	PlayAnimMontage(PrimaryAttackAnim);

	GetWorldTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_TimeElapsed, ProjectileSpawnDelayTime);
}

void ASCharacter::PrimaryAttack_TimeElapsed()
{
	// 获取骨骼插槽为"Muzzle_01"的坐标，这样子弹就不是从玩家中心点发射
	FVector HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
	FTransform SpwanTM = FTransform(CalcProjectileSpawnRotation(HandLocation), HandLocation);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = this;
	if (ensure(ClassToSpawn))
	{
		GetWorld()->SpawnActor<ASProjectileBase>(ClassToSpawn, SpwanTM, SpawnParams);
	}
}

void ASCharacter::PrimaryInteract()
{
	if (InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
}

void ASCharacter::SwitchMagicProjectile()
{
	ClassToSpawn = PrimaryAttackProjectile;
}

void ASCharacter::SwitchBlackholeProjectile()
{
	ClassToSpawn = SecondAttackProjectile;
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

FRotator ASCharacter::CalcProjectileSpawnRotation(FVector HandLocation)
{
	FCollisionObjectQueryParams ObjectQueryParams; 
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FVector TraceBeginLocation = CameraComp->GetComponentLocation();
	FVector TraceEndLocation = TraceBeginLocation + GetControlRotation().Vector() * 5000;

	FRotator SpawnRotation;

	FHitResult Hit;
	bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, TraceBeginLocation, TraceEndLocation, ObjectQueryParams);
	if (bBlockingHit)
	{
		TraceEndLocation = Hit.ImpactPoint;
	}

	// SpawnRotation =  FRotationMatrix::MakeFromX(TraceEndLocation - HandLocation).Rotator();
	SpawnRotation = FindLookAtRotation(TraceEndLocation - HandLocation);
	DrawDebugLine(GetWorld(), HandLocation, TraceEndLocation, FColor::Red, false, 2.0f, 0, 2.0f);
	return SpawnRotation;
}


