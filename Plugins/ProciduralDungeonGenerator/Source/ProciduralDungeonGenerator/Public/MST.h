#pragma once
#include <vector>
#include <unordered_set>

#include "DelaunayTriangulation.h"

using Edges = std::vector<DelaunayTriangle3D::Edge<UE::Math::TVector<double>>>;
using Edge = DelaunayTriangle3D::Edge<UE::Math::TVector<double>>;

namespace MST
{
	Edges MinimumSpanningTree(const Edges& EdgeList, FVector start)
	{
		TArray<FVector> OpenSet;
		TArray<FVector> CloseSet;

		for (auto edge : EdgeList)
		{
			OpenSet.Add(edge.p0);
			OpenSet.Add(edge.p1);
		}

		CloseSet.Add(start);

		Edges results;

		while (OpenSet.Num() > 0)
		{
			bool chosen = false;
			Edge chosenEdge = {{0,0,0}, {0,0,0}, 0};
			float minWeight = std::numeric_limits<float>::infinity();

			for (auto edge : EdgeList)
			{
				int closedVertices = 0;
				if (!CloseSet.Contains(edge.p0)) closedVertices++;
				if (!CloseSet.Contains(edge.p1)) closedVertices++;
				if (closedVertices != 1) continue;

				if (edge.Weight < minWeight) {
					chosenEdge = edge;
					chosen = true;
					minWeight = edge.Weight;
				}
			}

			if (!chosen) break;
			results.push_back(chosenEdge);
			OpenSet.Add(chosenEdge.p0);
			OpenSet.Add(chosenEdge.p1);
			CloseSet.Add(chosenEdge.p0);
			CloseSet.Add(chosenEdge.p1);
		}
		return results;
	};
}
