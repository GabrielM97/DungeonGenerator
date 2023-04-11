// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSpline.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

// Sets default values
ACustomSpline::ACustomSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	SetRootComponent(SplineComponent);
}


void ACustomSpline::CreatePath(UStaticMesh* Mesh, const float SectionLength, const FVector Start,
	const FVector End)
{
	if (!Mesh)
	{
		return;
	}

	SplineComponent->SetWorldLocationAtSplinePoint(
				0, Start);

	SplineComponent->SetWorldLocationAtSplinePoint(
				SplineComponent->GetNumberOfSplinePoints()-1, End);
	

	const int splineLegnth = SplineComponent->GetSplineLength();
	
	for (int splineCount = 0; splineCount <  trunc(splineLegnth/SectionLength); ++splineCount)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this,
											USplineMeshComponent::StaticClass());

		SplineMesh->SetStaticMesh(Mesh);
		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMesh->RegisterComponentWithWorld(GetWorld());
		SplineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		SplineMesh->SetForwardAxis(ESplineMeshAxis::X);
		
		const FVector StartPoint =	 SplineComponent->GetLocationAtDistanceAlongSpline(
									 splineCount * SectionLength, ESplineCoordinateSpace::Local);
		const FVector StartTangent = SplineComponent->GetLocationAtDistanceAlongSpline(
									 splineCount * SectionLength, ESplineCoordinateSpace::Local).GetClampedToSize(0, SectionLength);
		const FVector EndPoint =	 SplineComponent->GetLocationAtDistanceAlongSpline(
									 (splineCount+1) * SectionLength, ESplineCoordinateSpace::Local);
		const FVector EndTangent =	 SplineComponent->GetLocationAtDistanceAlongSpline(
									 (splineCount+1) * SectionLength, ESplineCoordinateSpace::Local).GetClampedToSize(0, SectionLength);;

		SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent);

		SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
	}
}

// Called when the game starts or when spawned
void ACustomSpline::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACustomSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

