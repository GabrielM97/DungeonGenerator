#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

namespace DelaunayTriangle3D
{
	constexpr double eps = 1e-4;

	template <typename T>
	struct Edge
	{
		using Node = FVector;
		Node p0, p1;
		int Weight;

		Edge(const Node& _p0, const Node& _p1, int weight) : p0{_p0}, p1{_p1}, Weight{weight}
		{
		}

		bool operator==(const Edge& other) const
		{
			return ((other.p0 == p0 && other.p1 == p1) ||
				(other.p0 == p1 && other.p1 == p0));
		}

		bool operator<(const Edge& other) const
		{
			return Weight < other.Weight;
		}
	};

	template <typename T>
	struct Circle
	{
		T x, y, radius;
		Circle() = default;
	};

	template <typename T>
	struct Triangle
	{
		using Node = FVector;
		Node p0, p1, p2;
		Edge<T> e0, e1, e2;
		Circle<float> circle;

		Triangle(const Node& _p0, const Node& _p1, const Node& _p2)
			: p0{_p0},
			  p1{_p1},
			  p2{_p2},
			  e0{_p0, _p1, (_p0 - _p1).Length()},
			  e1{_p1, _p2, (_p1 - _p2).Length()},
			  e2{_p0, _p2, (_p0 - _p2).Length()},
			  circle{}
		{
			const auto ax = p1.X - p0.X;
			const auto ay = p1.Y - p0.Y;
			const auto bx = p2.X - p0.X;
			const auto by = p2.Y - p0.Y;

			const auto m = p1.X * p1.X - p0.X * p0.X + p1.Y * p1.Y - p0.Y * p0.Y;
			const auto u = p2.X * p2.X - p0.X * p0.X + p2.Y * p2.Y - p0.Y * p0.Y;
			const auto s = 1. / (2. * (ax * by - ay * bx));

			circle.x = ((p2.Y - p0.Y) * m + (p0.Y - p1.Y) * u) * s;
			circle.y = ((p0.X - p2.X) * m + (p1.X - p0.X) * u) * s;

			const auto dx = p0.X - circle.x;
			const auto dy = p0.Y - circle.y;
			circle.radius = dx * dx + dy * dy;
		}
	};

	template <typename T>
	struct Delaunay
	{
		std::vector<Triangle<T>> triangles;
		std::vector<Edge<T>> edges;
	};

	template <typename T>
	Delaunay<T> triangulate(const TArray<FVector>& points)
	{
		using Node = FVector;
		if (points.Num() < 3)
		{
			return Delaunay<T>{};
		}
		auto xmin = points[0].X;
		auto xmax = xmin;
		auto ymin = points[0].Y;
		auto ymax = ymin;
		for (const auto& pt : points)
		{
			xmin = FMath::Min(xmin, pt.X);
			xmax = FMath::Max(xmax, pt.X);
			ymin = FMath::Min(ymin, pt.Y);
			ymax = FMath::Max(ymax, pt.Y);}

		const auto dx = xmax - xmin;
		const auto dy = ymax - ymin;
		const auto dmax = std::max(dx, dy);
		const auto midx = (xmin + xmax) / 2.;
		const auto midy = (ymin + ymax) / 2.;

		/* Init Delaunay triangulation. */
		auto d = Delaunay<T>{};

		const auto p0 = Node(midx - 20 * dmax, midy - dmax, 0.);
		const auto p1 = Node(midx, midy + 20 * dmax, 0);
		const auto p2 = Node(midx + 20 * dmax, midy - dmax, 0);
		d.triangles.push_back(Triangle<T>{p0, p1, p2});

		for (const auto& pt : points)
		{
			std::vector<Edge<T>> edges;
			std::vector<Triangle<T>> tmps;
			for (const auto& tri : d.triangles)
			{
				/* Check if the point is inside the triangle circumcircle. */
				const auto dist = (tri.circle.x - pt.X) * (tri.circle.x - pt.X) +
					(tri.circle.y - pt.Y) * (tri.circle.y - pt.Y);
				if ((dist - tri.circle.radius) <= eps)
				{
					edges.push_back(tri.e0);
					edges.push_back(tri.e1);
					edges.push_back(tri.e2);
				}
				else
				{
					tmps.push_back(tri);
				}
			}

			/* Delete duplicate edges. */
			std::vector<bool> remove(edges.size(), false);
			for (auto it1 = edges.begin(); it1 != edges.end(); ++it1)
			{
				for (auto it2 = edges.begin(); it2 != edges.end(); ++it2)
				{
					if (it1 == it2)
					{
						continue;
					}
					if (*it1 == *it2)
					{
						remove[std::distance(edges.begin(), it1)] = true;
						remove[std::distance(edges.begin(), it2)] = true;
					}
				}
			}

			edges.erase(
				std::remove_if(edges.begin(), edges.end(),
							   [&](const auto& e) { return remove[&e - &edges[0]]; }),
				edges.end());

			/* Update triangulation. */
			for (const auto& e : edges)
			{
				tmps.push_back({e.p0, e.p1, {pt.X, pt.Y, 0}});
			}
			d.triangles = tmps;
		}

		/* Remove original super triangle. */
		d.triangles.erase(
			std::remove_if(d.triangles.begin(), d.triangles.end(),
						   [&](const auto& tri)
						   {
							   return ((tri.p0 == p0 || tri.p1 == p0 || tri.p2 == p0) ||
								   (tri.p0 == p1 || tri.p1 == p1 || tri.p2 == p1) ||
								   (tri.p0 == p2 || tri.p1 == p2 || tri.p2 == p2));
						   }),
			d.triangles.end());

		/* Add edges. */
		for (const auto& tri : d.triangles)
		{
			d.edges.push_back(tri.e0);
			d.edges.push_back(tri.e1);
			d.edges.push_back(tri.e2);
		}
		return d;
	}
}