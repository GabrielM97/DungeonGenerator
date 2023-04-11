// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"

#include "CustomSpline.h"
#include "DelaunayTriangulation.h"
#include "MST.h"
#include "StaticMeshAttributes.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "CompGeom/Delaunay2.h"
#include "CompGeom/Delaunay3.h"

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADungeonGenerator::GenerateDungeon()
{
	if (const auto World = GetWorld())
	{
		//Spawn Cells
		for (int CellSpawned = 0; CellSpawned < NumberOfCells; ++CellSpawned)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FVector Location = GetRandomPointInCircle(SpawnRadius);
			auto Cell = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
			Cell->SetActorScale3D(FVector(FMath::RandRange(MinSize, MaxSize),
											FMath::RandRange(MinSize, MaxSize),
											0.5));
			Cell->GetStaticMeshComponent()->SetStaticMesh(RoomMesh);
			SpawnedCells.Add(Cell);
		}
		
		for (const auto Cell : SpawnedCells)
		{
			//Separate
			auto Vel = Separate(Cell);
			int Attempts = 0;
			while (Vel != FVector::ZeroVector && Attempts < 10)
			{
				Cell->SetActorLocation(Cell->GetActorLocation() + Vel);
				Vel = Separate(Cell);
				++Attempts;
			}

			//Determine which cells can be rooms

			if (Cell->GetActorScale().X > MinSize+5 && Cell->GetActorScale().Y > MinSize+5)
			{
				UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(Cell->GetStaticMeshComponent()->GetMaterial(0), NULL);
				material->SetVectorParameterValue(FName(TEXT("SurfaceColor")), FLinearColor(0.9f, 0.1f, 0.1f));
				Cell->GetStaticMeshComponent()->SetMaterial(0, material);

				//SpawnedCells.Remove(Cell);
				Rooms.Add(Cell->GetActorLocation());
			}
		}

		auto DT = DelaunayTriangle3D::triangulate<FVector>(Rooms);
		auto MST = MST::MinimumSpanningTree(DT.edges, DT.edges[0].p0);
		
		for (auto Edge : DT.edges)
		{
			if (UKismetMathLibrary::RandomFloat() > 0.9)
			{
				MST.push_back(Edge);
			}
		}

		for (auto Edge : MST)
		{
			FVector pathLoc = Edge.p1 - Edge.p0;
			int count = trunc(pathLoc.X/SectionLegnth);
			int dir = count < 0 ? -1:1;
			for (int pathCountX = 0; pathCountX < abs(count) + 1 ; pathCountX++)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
				FVector Location = FVector(Edge.p0.X + pathCountX*SectionLegnth * dir, Edge.p0.Y, -35);
				auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
				                                                     FRotator::ZeroRotator, SpawnParams);
				path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
				SpawnedPath.Add(path);
			}

			count = trunc(pathLoc.Y/SectionLegnth);
			dir = count < 0 ? 1:-1;
			for (int pathCountY = 0; pathCountY < abs(count) + 1; pathCountY++)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
				FVector Location = FVector(Edge.p1.X, Edge.p1.Y + pathCountY*SectionLegnth * dir, -35);
				auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
																	 FRotator::ZeroRotator, SpawnParams);
				path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
				SpawnedPath.Add(path);
			}
			
			
			//DrawDebugLine(GetWorld(), Edge.p0, Edge.p1, FColor::Yellow, true);
		}
	}
}

void ADungeonGenerator::ClearDungeon()
{
	for (const auto Cell : SpawnedCells)
	{
		Cell->Destroy();
	}

	for (const auto Path : SpawnedPath)
	{
		Path->Destroy();
	}

	SpawnedCells.Empty();
	Rooms.Empty();
	SpawnedPath.Empty();
	FlushPersistentDebugLines(GetWorld());
}

// Called every frame
void ADungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ADungeonGenerator::Separate(AStaticMeshActor* CurrentCell)
{
	FVector Velocity = FVector::ZeroVector;
	int NeighborCount = 0;

	for (const auto Cell : SpawnedCells)
	{
		if (Cell != CurrentCell)
		{
			FVector currentLoc = CurrentCell->GetActorLocation();
			FVector targetLoc = Cell->GetActorLocation();
			const float Distance = FVector::DistXY(currentLoc, targetLoc);
			
			const float scale = CurrentCell->GetActorScale().Length()*100;
			
			if (Distance < MaxDistance)
			{
				Velocity += (currentLoc - targetLoc)/CurrentCell->GetActorScale();
				NeighborCount++;
			}
		}
	}

	if (NeighborCount == 0)
	{
		return FVector::ZeroVector;
	}

	Velocity /= NeighborCount;
	//Velocity *= -1;
	Velocity.Normalize(1);
	return Velocity*10000;
}

FVector ADungeonGenerator::GetRandomPointInCircle(float radius)
{
	float t = 2*UKismetMathLibrary::GetPI()*UKismetMathLibrary::RandomFloat();
	float u = UKismetMathLibrary::RandomFloat() + UKismetMathLibrary::RandomFloat();
	float r = u > 1 ? 2-u: u;

	return FVector(radius*r*UKismetMathLibrary::Cos(t), radius*r*UKismetMathLibrary::Sin(t), 0);
}

