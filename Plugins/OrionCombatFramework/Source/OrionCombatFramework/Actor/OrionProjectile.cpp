// Copyright 2025 Ivan Chandra. All Rights Reserved.


#include "../Actor/OrionProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AOrionProjectile::AOrionProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
    SetRootComponent(Sphere);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
    ProjectileMovement->InitialSpeed = 550.f;
    ProjectileMovement->MaxSpeed = 550.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AOrionProjectile::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifeSpan);
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AOrionProjectile::OnSphereOverlap);
    LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AOrionProjectile::Destroyed()
{
    if (!bHit && !HasAuthority())
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
        LoopingSoundComponent->Stop();
    }
    Super::Destroyed();
}

void AOrionProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
    LoopingSoundComponent->Stop();

    if (HasAuthority())
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
        }
        Destroy();
    }
    else
    {
        bHit = true;
    }
}
