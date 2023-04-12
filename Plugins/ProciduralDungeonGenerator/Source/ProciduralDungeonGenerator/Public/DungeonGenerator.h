// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DelaunayTriangulation.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"

using DGEdge = DelaunayTriangle3D::Edge<UE::Math::TVector<double>>;

struct Bounds
{
	FVector Origin;
	FVector Extent;

	bool Overlap(FVector pos)
	{
		return (pos.X >= Origin.X - Extent.X && pos.X <= Origin.X + Extent.X) &&
			  (pos.Y >= Origin.Y - Extent.Y && pos.Y <= Origin.Y + Extent.Y);
	}
};

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
	UStaticMesh* RoomMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	UStaticMesh* PathMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	float SectionLegnth{100};
	
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

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Dungeon Generation")
	int SnapSize{5};
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	TArray<class AStaticMeshActor*> SpawnedCells;
	TArray<class AStaticMeshActor*> SpawnedPath;
	TArray<FVector> Rooms;

	FVector Separate(AStaticMeshActor* CurrentCell);

	FVector GetRandomPointInCircle(float radius);
	int RoundM(float loc, int snapSize);
	FVector RoundM(FVector loc, int snapSize);
	Bounds GetRoomExtentByLocation(FVector Location);
	DGEdge GetClosestEdge(FVector start, FVector end);
};
