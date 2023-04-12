// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "CustomSpline.h"
#include "DelaunayTriangulation.h"
#include "MST.h"
#include "StaticMeshAttributes.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

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
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			FVector Location = GetRandomPointInCircle(SpawnRadius);
			auto Cell = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
			                                                FRotator::ZeroRotator, SpawnParams);
			Cell->SetActorScale3D(FVector(FMath::RandRange(MinSize, MaxSize),
			                              FMath::RandRange(MinSize, MaxSize),
			                              0.5));
			Cell->GetStaticMeshComponent()->SetStaticMesh(RoomMesh);
			SpawnedCells.Add(Cell);
		}

		for (const auto Cell : SpawnedCells)
		{
			//Separate
			auto Vel = RoundM(Separate(Cell), SnapSize);
			int Attempts = 0;
			while (Vel != FVector::ZeroVector && Attempts < 10)
			{
				Cell->SetActorLocation(Cell->GetActorLocation() + Vel);
				Vel = RoundM(Separate(Cell), SnapSize);
				++Attempts;
			}

			//Determine which cells can be rooms

			if (Cell->GetActorScale().X > MinSize + 5 && Cell->GetActorScale().Y > MinSize + 5)
			{
				UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(
					Cell->GetStaticMeshComponent()->GetMaterial(0), NULL);
				material->SetVectorParameterValue(FName(TEXT("SurfaceColor")), FLinearColor(0.9f, 0.1f, 0.1f));
				Cell->GetStaticMeshComponent()->SetMaterial(0, material);

				//SpawnedCells.Remove(Cell);
				Rooms.Add(Cell->GetActorLocation());
				
			}else if(UKismetMathLibrary::RandomFloat() > 0.85)
			{
				Rooms.Add(Cell->GetActorLocation());
			}
			else
			{
				Cell->GetStaticMeshComponent()->SetVisibility(false);
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

			Bounds b0 = GetRoomExtentByLocation(Edge.p0);
			Bounds b1 = GetRoomExtentByLocation(Edge.p1);

			DGEdge dgedge = GetClosestEdge(Edge.p0, Edge.p1);
			
			FVector pathLoc = dgedge.p1 - dgedge.p0;
			int countX = trunc(pathLoc.X / SectionLegnth);
			int countY = trunc(pathLoc.Y / SectionLegnth);
			int dirX = countX < 0 ? -1 : 1;
			int dirY = countY < 0 ? -1 : 1;

			int TotalBlocksToSpawn = abs(countX) + abs(countY) + 1;

			FVector Location = dgedge.p0;
			
			while (TotalBlocksToSpawn > 0)
			{
				//spawn Y
				if (TotalBlocksToSpawn <= abs(countY))
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride =
						ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
				
					Location.Y += SectionLegnth*dirY;
					
					auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
																	 FRotator::ZeroRotator, SpawnParams);
					path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
					SpawnedPath.Add(path);
				
					
					--TotalBlocksToSpawn;
					continue;
				}

				//spawn x
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride =
					ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
				
				Location.X += SectionLegnth*dirX;
				
				auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
																 FRotator::ZeroRotator, SpawnParams);
				path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
				SpawnedPath.Add(path);
				

				--TotalBlocksToSpawn;
			}
			
			// for (int pathCountX = 0; pathCountX < abs(count)+1; pathCountX++)
			// {
			// 	FActorSpawnParameters SpawnParams;
			// 	SpawnParams.SpawnCollisionHandlingOverride =
			// 		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			// 	FVector Location = RoundM(FVector(dgedge.p0.X  + (pathCountX * SectionLegnth * dir), dgedge.p0.Y, 0), SnapSize);
			// 	if (!b0.Overlap(Location) && !b1.Overlap(Location))
			// 	{
			// 		auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
			// 														 FRotator::ZeroRotator, SpawnParams);
			// 		path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
			// 		SpawnedPath.Add(path);
			// 	}
			// 	
			// }
			//
			// count = trunc(pathLoc.Y / SectionLegnth);
			// dir = count < 0 ? 1 : -1;
			// for (int pathCountY = 0; pathCountY < abs(count)+1; pathCountY++)
			// {
			// 	FActorSpawnParameters SpawnParams;
			// 	SpawnParams.SpawnCollisionHandlingOverride =
			// 		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			// 	FVector Location = RoundM(FVector(dgedge.p1.X, dgedge.p1.Y + pathCountY * SectionLegnth * dir, 0), SnapSize);
			// 	if (!b0.Overlap(Location) && !b1.Overlap(Location))
			// 	{
			// 		auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
			// 														 FRotator::ZeroRotator, SpawnParams);
			// 		path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
			// 		SpawnedPath.Add(path);
			// 	}
			// }


			DrawDebugLine(GetWorld(), Edge.p0, Edge.p1, FColor::Yellow, true);
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

			const float scale = CurrentCell->GetActorScale().Length() * 100;

			if (Distance < MaxDistance)
			{
				Velocity += (currentLoc - targetLoc) / CurrentCell->GetActorScale();
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
	return Velocity * 10000;
}

FVector ADungeonGenerator::GetRandomPointInCircle(float radius)
{
	float t = 2 * UKismetMathLibrary::GetPI() * UKismetMathLibrary::RandomFloat();
	float u = UKismetMathLibrary::RandomFloat() + UKismetMathLibrary::RandomFloat();
	float r = u > 1 ? 2 - u : u;

	return FVector(RoundM(radius * r * UKismetMathLibrary::Cos(t), SnapSize),
	               RoundM(radius * r * UKismetMathLibrary::Sin(t), SnapSize), 0);
}

int ADungeonGenerator::RoundM(float loc, int snapSize)
{
	return FMath::Floor(((loc + SnapSize - 1) / SnapSize)) * SnapSize;
}


FVector ADungeonGenerator::RoundM(FVector loc, int snapSize)
{
	return FVector(FMath::Floor(((loc.X + SnapSize - 1) / SnapSize)) * SnapSize,
								FMath::Floor(((loc.Y + SnapSize - 1) / SnapSize)) * SnapSize,
								0) ;
}


Bounds ADungeonGenerator::GetRoomExtentByLocation(FVector Location)
{
	const auto room = *SpawnedCells.FindByPredicate(
			[Location](const AStaticMeshActor* MeshActor){return MeshActor->GetActorLocation() == Location;});
	Bounds bounds;
	room->GetActorBounds(true, bounds.Origin, bounds.Extent);
	return bounds;
}

DGEdge ADungeonGenerator::GetClosestEdge(FVector start, FVector end)
{
	Bounds b0 = GetRoomExtentByLocation(start);
	Bounds b1 = GetRoomExtentByLocation(end);
	
	TArray<DGEdge> sortarr;

	//start bound
	FVector p01 = FVector( start.X ,start.Y - b0.Extent.Y, start.Z);
	FVector p02 = FVector( start.X + b0.Extent.X, start.Y, start.Z);
	FVector p03 = FVector( start.X ,start.Y + b0.Extent.Y, start.Z);
	FVector p04 = FVector( start.X - b0.Extent.X, start.Y, start.Z);

	//end bounds
	FVector p11 = FVector( end.X ,end.Y - b1.Extent.Y, end.Z);
	FVector p12 = FVector( end.X + b1.Extent.X, end.Y, end.Z);
	FVector p13 = FVector( end.X ,end.Y + b1.Extent.Y, end.Z);
	FVector p14 = FVector( end.X - b1.Extent.X, end.Y, end.Z);
	
	// FVector P02P14 = FVector(end.X - b1.Extent.X, end.Y, end.Z) - FVector( start.X + b0.Extent.X, start.Y, start.Z);
	// FVector P02P13 = FVector(end.X, end.Y + b1.Extent.Y, end.Z) - FVector( start.X + b0.Extent.X, start.Y, start.Z);
	// FVector P01P14 = FVector(end.X - b1.Extent.X, end.Y, end.Z) - FVector( start.X ,start.Y - b0.Extent.Y, start.Z);
	// FVector P01P13 = FVector(end.X, end.Y + b1.Extent.Y, end.Z) - FVector( start.X, start.Y - b0.Extent.Y, start.Z);

	sortarr.Add({p01, p11, static_cast<int>(FVector::Distance(p11, p01))});
	sortarr.Add({p01, p12, static_cast<int>(FVector::Distance(p12, p01))});
	sortarr.Add({p01, p13, static_cast<int>(FVector::Distance(p13, p01))});
	sortarr.Add({p01, p14, static_cast<int>(FVector::Distance(p14, p01))});
	
	sortarr.Add({p02, p11, static_cast<int>(FVector::Distance(p11, p02))});
	sortarr.Add({p02, p12, static_cast<int>(FVector::Distance(p12, p02))});
	sortarr.Add({p02, p13, static_cast<int>(FVector::Distance(p13, p02))});
	sortarr.Add({p02, p14, static_cast<int>(FVector::Distance(p14, p02))});
	
	sortarr.Add({p03, p11, static_cast<int>(FVector::Distance(p11, p03))});
	sortarr.Add({p03, p12, static_cast<int>(FVector::Distance(p12, p03))});
	sortarr.Add({p03, p13, static_cast<int>(FVector::Distance(p13, p03))});
	sortarr.Add({p03, p14, static_cast<int>(FVector::Distance(p14, p03))});
	
	sortarr.Add({p04, p11, static_cast<int>(FVector::Distance(p11, p04))});
	sortarr.Add({p04, p12, static_cast<int>(FVector::Distance(p12, p04))});
	sortarr.Add({p04, p13, static_cast<int>(FVector::Distance(p13, p04))});
	sortarr.Add({p04, p14, static_cast<int>(FVector::Distance(p14, p04))});

	sortarr.Sort();

	return sortarr[0];
}