// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"

UCLASS()
class PROCIDURALDUNGEONGENERATOR_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	UStaticMesh* MeshObj;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	int NumberOfCells;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Dungeon Generation")
	void GenerateDungeon();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Dungeon Generation")
	void ClearDungeon();
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	int MinSize;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	int MaxSize;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	float SpawnRadius;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	float MaxDistance;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	TArray<class AStaticMeshActor*> SpawnedCells;
	TArray<FVector> Rooms;

	FVector Separate(AStaticMeshActor* CurrentCell);

	FVector GetRandomPointInCircle(float radius);
	
};
