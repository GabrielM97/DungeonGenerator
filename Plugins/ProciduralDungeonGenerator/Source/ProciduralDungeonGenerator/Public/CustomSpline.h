// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomSpline.generated.h"

UCLASS()
class PROCIDURALDUNGEONGENERATOR_API ACustomSpline : public AActor
{
	
private:
	GENERATED_BODY()

public:	
	//Sets default values for this actor's properties
	ACustomSpline();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spline")
	class USplineComponent* SplineComponent;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spline")
	// UStaticMesh* Mesh;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spline")
	// float SectionLength;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spline")
	// FVector SplineEnd;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spline")
	// FVector SplineStart;

	void CreatePath(UStaticMesh* Mesh, const float SectionLength, const FVector Start, const FVector End);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
