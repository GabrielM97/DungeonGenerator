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
		bIsDungeonGenerating = true;
		//Spawn Cells
		for (int CellSpawned = 0; CellSpawned < NumberOfCells; ++CellSpawned)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			FVector Location = GetRandomPointInCircle(SpawnRadius);
			auto Cell = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
			                                                FRotator::ZeroRotator, SpawnParams);
			Cell->SetActorScale3D(FVector(FMath::RandRange(MinSize, MaxSize)*2,
			                              FMath::RandRange(MinSize, MaxSize)*2,
			                              2));
			
			Cell->SetMobility(EComponentMobility::Movable);
			Cell->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Cell->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Cell->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldDynamic);
			Cell->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
			Cell->GetStaticMeshComponent()->SetStaticMesh(RoomMesh);
			SpawnedCells.Add(Cell);
		}
		
		bIsSeparating = true;
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

	if (bIsDungeonGenerating)
	{
		if (!bIsSeparating)
		{
			for (const auto Cell : SpawnedCells)
			{
				if (Cell->GetActorScale().X > MinSize + 10 && Cell->GetActorScale().Y > MinSize + 10)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(
						Cell->GetStaticMeshComponent()->GetMaterial(0), NULL);
					material->SetVectorParameterValue(FName(TEXT("SurfaceColor")), FLinearColor(0.9f, 0.1f, 0.1f));
					Cell->GetStaticMeshComponent()->SetMaterial(0, material);
			
					Cell->SetActorLocation(RoundM(Cell->GetActorLocation(), SnapSize));
					Rooms.Add(Cell->GetActorLocation());
					
				
				}else if(UKismetMathLibrary::RandomFloat() > 0.85)
				{
					UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(
						Cell->GetStaticMeshComponent()->GetMaterial(0), NULL);
					material->SetVectorParameterValue(FName(TEXT("SurfaceColor")), FLinearColor(0.9f, 0.1f, 0.1f));
					Cell->GetStaticMeshComponent()->SetMaterial(0, material);

					Cell->SetActorLocation(RoundM(Cell->GetActorLocation(), SnapSize));
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
			
			//Gen Pathways;
			for (auto Edge : MST)
			{

				//DGEdge dgEdge = GetClosestEdge(Edge.p0, Edge.p1);
				
				FVector pathLoc = Edge.p1 - Edge.p0;
				int countX = trunc(pathLoc.X / SectionLegnth);
				int countY = trunc(pathLoc.Y / SectionLegnth);
				int dirX = countX < 0 ? -1 : 1;
				int dirY = countY < 0 ? -1 : 1;

				countX = abs(countX) + (dirX == 1 ? 0 : 1);
				countY = abs(countY) + (dirY == 1 ? 0 : 1);
				
				int TotalBlocksToSpawn = abs(countX) + abs(countY) ;
			
				FVector Location;
				Location.X = Edge.p0.X + SectionLegnth * dirX;
				Location.Y = Edge.p0.Y + SectionLegnth/2 ;
				Location.Z = -5;
			
				while (TotalBlocksToSpawn > 0)
				{
					//spawn Y
					if (TotalBlocksToSpawn <= abs(countY))
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.bNoFail = false;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
				
						Location.Y += SectionLegnth*dirY;
						//DrawDebugBox(GetWorld(), RoundM(Location, SnapSize), FVector(SectionLegnth/2), FColor::Blue, true);
						if (!IsOverlappingRoom(Location) && !SpawnedPath.ContainsByPredicate(
							[&](AStaticMeshActor* path){return Location == path->GetActorLocation();}))
						{
							auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
																	 FRotator::ZeroRotator, SpawnParams);
						
							path->SetMobility(EComponentMobility::Movable);
							path->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
							path->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
							path->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldDynamic);
							path->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
							path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
							SpawnedPath.Add(path);
						}
						--TotalBlocksToSpawn;
						continue;
					}
			
					//spawn x
					FActorSpawnParameters SpawnParams;
					SpawnParams.bNoFail = false;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
				
					Location.X += SectionLegnth*dirX;
					//DrawDebugBox(GetWorld(),  RoundM(Location, SnapSize), FVector(SectionLegnth/2), FColor::Blue, true);
					if (!IsOverlappingRoom(Location) && !SpawnedPath.ContainsByPredicate(
						[&](AStaticMeshActor* path){return Location == path->GetActorLocation();}))
					{
						auto path = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location,
																 FRotator::ZeroRotator, SpawnParams);
						
						path->SetMobility(EComponentMobility::Movable);
						path->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
						path->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						path->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldDynamic);
						path->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
						path->GetStaticMeshComponent()->SetStaticMesh(PathMesh);
						SpawnedPath.Add(path);
					}
					--TotalBlocksToSpawn;
				}
			
				//DrawDebugLine(GetWorld(), Edge.p0, Edge.p1, FColor::Yellow, true);
			}

			bIsDungeonGenerating = false;
		}
		else
		{
			FVector Vel = FVector::ZeroVector;
			for (const auto Cell : SpawnedCells)
			{
				//Separate
				
				const FVector Force = Separate(Cell);
				Vel += Force;
				Cell->SetActorLocation(Cell->GetActorLocation() + Force);
				//Determine which cells can be rooms
			}

			bIsSeparating = Vel != FVector::ZeroVector;
		}
	}
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

			if (Distance == 0)
			{
				NeighborCount = 1;
				Velocity = FVector(MinDistance, MinDistance, 0.);
			}
			
			if (Distance <= MinDistance)
			{
				Bounds b;
				CurrentCell->GetActorBounds(true, b.Origin, b.Extent);
				Velocity += (currentLoc - targetLoc) / b.Extent.Length();
				NeighborCount++;
			}
		}
	}

	if (NeighborCount == 0)
	{
		return FVector::ZeroVector;
	}

	Velocity /= NeighborCount;
	Velocity.Normalize(1);
	return Velocity*100;
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

bool ADungeonGenerator::IsOverlappingRoom(FVector loc)
{
	TArray<AStaticMeshActor*> rooms = SpawnedCells.FilterByPredicate([](AStaticMeshActor* cell)
											  {return cell->GetStaticMeshComponent()->IsVisible(); });
	
	for (auto Room : rooms)
	{
		Bounds b;

		Room->GetActorBounds(true, b.Origin, b.Extent);

		if(b.Overlap(loc))
		{
			return true;
		}
	}

	return false;
}
