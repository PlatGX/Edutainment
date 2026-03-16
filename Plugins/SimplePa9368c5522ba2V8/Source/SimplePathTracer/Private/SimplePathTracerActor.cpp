// Copyright 2023 Andrew Esenin. All right reserved.

#include "SimplePathTracerActor.h"
#include "Curves/CurveLinearColor.h"

ASimplePathTracerActor::ASimplePathTracerActor()
{
	
}

bool ASimplePathTracerActor::GetPathData(const TArray<FVector>& PathPoints,
                                         TArray<FVector>& Vertexes,
                                         TArray<int>& Triangles,
                                         TArray<FVector2D>& UV,
                                         const bool bLoopPath,
                                         const float Thickness,
                                         const float Offset,
                                         const bool bEnableUV,
                                         const FVector2D ScaleUV,
                                         const FVector2D OffsetUV,
                                         const bool bRemoveSeamsOnUV,
                                         const bool bRectangularUV)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || Thickness == 0) return false;
	
	const bool bCanLoop = bLoopPath && NumPoints > 2;
	
	TArray<FVector2D> Points2D;
	TArray<float> PointsZ;
	TArray<float> SinAngles;
	TArray<FVector2D> Normals;
	
	TArray<FVector> Vertexes3D;
	TArray<FVector2D> Vertexes2D;
	TArray<float> VertexesZ;

	SplitVectorArray(PathPoints, Points2D, PointsZ);

	CalculatePathNormals(Points2D, Normals, SinAngles);
	
	if(bCanLoop)	FindVertexesLoop(Points2D, PointsZ, SinAngles, Normals, bEnableUV, Thickness, Offset, Vertexes2D, VertexesZ);
	else			FindVertexes	(Points2D, PointsZ, SinAngles, Normals, bEnableUV, Thickness, Offset, Vertexes2D, VertexesZ);

	const int NumVertexes = Vertexes2D.Num();
	
	if (NumVertexes < 3) return false;
	
	Triangles = CalculateTriangles(NumVertexes, bEnableUV);

	if(bEnableUV)
	{
		if (bRectangularUV)
		{
			UV = CalculateSimpleUV(Vertexes2D, Thickness, ScaleUV, OffsetUV, bRemoveSeamsOnUV);
		}
		else
		{
			UV = CalculateUV(Vertexes2D, Thickness, ScaleUV, OffsetUV, bRemoveSeamsOnUV);
		}
	}
	
	CombineVectorArray(Vertexes2D, VertexesZ, Vertexes3D);
	
	Vertexes = Vertexes3D;
	
	return true;
}

bool ASimplePathTracerActor::GetVerticalPathData(const TArray<FVector>& PathPoints,
                                         TArray<FVector>& Vertexes,
                                         TArray<int>& Triangles,
                                         TArray<FVector2D>& UV,
                                         const bool bLoopPath,
                                         const float Height,
                                         const float Offset,
                                         const bool bEnableUV)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || Height == 0) return false;

	TArray<FVector> Vertexes3D;

	const bool bCanLoop = bLoopPath && NumPoints > 2;
	
	FindVerticalVertexes(PathPoints, bCanLoop, bEnableUV, Height, Offset, Vertexes3D);
		
	Vertexes = Vertexes3D;
	
	const int NumVertexes = Vertexes3D.Num();
	
	if (NumVertexes < 3) return false;
	
	Triangles = CalculateTriangles(NumVertexes, bEnableUV);

	if (!bEnableUV) return true;

	TArray<FVector2D> Vertexes2D;
	Vertexes2D.Reserve(NumVertexes);
	
	for (int i = 0; i < NumVertexes; ++i)
	{
		const FVector& Vert = Vertexes3D[i];
		Vertexes2D.Add(FVector2D(Vert.X, Vert.Y));
	}
	
	UV = CalculateHeightUV(Vertexes2D, Height);
	
	return true;
}

bool ASimplePathTracerActor::GetDottedPathData(const TArray<FVector>& PathPoints,
                                               TArray<FVector>& Vertexes,
                                               TArray<int>& Triangles,
                                               TArray<FVector2D>& UV,
                                               const bool bLoopPath,
                                               const float Interval,
                                               const float Length,
                                               const float Width,
                                               const int DotDirection,
                                               const bool bEnableUV,
                                               const FVector2D ScaleUV,
                                               const FVector2D OffsetUV,
                                               const bool AddLastDot)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || Length == 0 || Width == 0) return false;

	TArray<FVector> NewPathPoints;
	TArray<FVector> DotCenters;
	TArray<FVector> Directions;

	if(bLoopPath)
	{
		NewPathPoints = PathPoints;
		NewPathPoints.Add(PathPoints[0]);
	}
	else
	{
		NewPathPoints = PathPoints;
	}
	
	GetPointsAlongPathDirected(NewPathPoints, DotCenters, Directions, Interval, AddLastDot);

	const int NumDots = DotCenters.Num();
	
	if (NumDots < 1) return false;
	
	for (int i = 0; i < NumDots; ++i)
	{
		CalculateQuad(DotCenters[i], Directions[i], Length, Width, bEnableUV, ScaleUV, DotDirection,
		              OffsetUV, Vertexes, Triangles, UV);
	}
	
	return true;
}

bool ASimplePathTracerActor::GetPathWithCornersOffsetData(const TArray<FVector>& PathPoints,
                                                          TArray<FVector>& Vertexes,
                                                          TArray<int>& Triangles,
                                                          TArray<FVector2D>& UV,
                                                          const bool bLoopPath,
                                                          const float Thickness,
                                                          const float Offset,
                                                          const bool bEnableUV,
                                                          const int DirectionUV,
                                                          const FVector2D ScaleUV,
                                                          const FVector2D OffsetUV)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2) return false;

	TArray<FVector> NewPathPoints;

	if(bLoopPath)
	{
		NewPathPoints = PathPoints;
		NewPathPoints.Add(PathPoints[0]);
	}
	else
	{
		NewPathPoints = PathPoints;
	}
	
	NewPathPoints = AddPathCornersOffset(NewPathPoints, Offset);

	const int NumSegments = NewPathPoints.Num() / 2;
	
	if (NumSegments < 1) return false;
	
	for (int i = 0; i < NumSegments; ++i)
	{
		const int j = i * 2; 
		const FVector& P1 = NewPathPoints[j];
		const FVector& P2 = NewPathPoints[j + 1];
		
		FVector V1 = P2 - P1;
		FVector Direction = V1.GetSafeNormal();
		FVector Center = V1 * 0.5f + P1;
		const float Length = V1.Size();
		
		CalculateQuad(Center, Direction, Length, Thickness, bEnableUV, ScaleUV, 0,
		              OffsetUV, Vertexes, Triangles, UV);
	}
	
	return true;
}

void ASimplePathTracerActor::GetStartPathSegment(const TArray<FVector>& PathPoints, const float StartOffset, bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction)
{
	const int NumPoints = PathPoints.Num();

	if(NumPoints == 0) return;
	if(NumPoints == 1)
	{
		Location = PathPoints[0];
		return;
	}

	const FVector& StartPoint = PathPoints[0];

	if (bIn2D)
	{
		Direction = PathPoints[1] - StartPoint;
		Direction.Z = 0.0f;
		Direction = Direction.GetSafeNormal();
	}
	else
	{
		Direction = (PathPoints[1] - StartPoint).GetSafeNormal();
	}

	Location = Direction * StartOffset + StartPoint;
	
	Rotation = FRotationMatrix::MakeFromX(-Direction).Rotator();
}

void ASimplePathTracerActor::GetEndPathSegment(const TArray<FVector>& PathPoints, const float EndOffset, bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction)
{
	const int NumPoints = PathPoints.Num();

	if(NumPoints == 0) return;
	if(NumPoints == 1)
	{
		Location = PathPoints[0];
		return;
	}

	const FVector& EndPoint = PathPoints[NumPoints - 1];

	if(bIn2D)
	{
		Direction = EndPoint - PathPoints[NumPoints - 2];
		Direction.Z = 0.0f;
		Direction = Direction.GetSafeNormal();
	}
	else
	{
		Direction = (EndPoint - PathPoints[NumPoints - 2]).GetSafeNormal();
	}

	Location = Direction * EndOffset + EndPoint;
	
	Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
}

TArray<FVector> ASimplePathTracerActor::RoundingPathCorners(const TArray<FVector>& PathPoints, const float Radius, const int Segments, const bool bLoopPath, const bool bPath3D)
{
	const int NumPoints = PathPoints.Num();
	
	if(NumPoints < 3 || Radius <= 0.f || Segments <= 0) return PathPoints;

	TArray<FVector> NewPoints;

	// For 3D
	if (bPath3D)
	{
		int Num = NumPoints;

		if (!bLoopPath)
		{
			Num -= 2;
			
			// Add First Point
			NewPoints.Add(PathPoints[0]);
		}
		
		// Find Corners Points
		for (int i = 0; i < Num; ++i)
		{
			int PreviousIndex;
			int CurrentIndex;
			int NextIndex;

			if (bLoopPath)
			{
				PreviousIndex = (i + NumPoints - 1) % NumPoints;
				CurrentIndex = i % NumPoints;
				NextIndex = (i + 1) % NumPoints;
			}
			else
			{
				PreviousIndex = i;
				CurrentIndex = (i + 1) % NumPoints;
				NextIndex = (i + 2) % NumPoints;
			}
			
			const FVector& A = PathPoints[PreviousIndex];
			const FVector& B = PathPoints[CurrentIndex];
			const FVector& C = PathPoints[NextIndex];
			
			const FVector BA = A - B;
			const FVector BC = C - B;

			const float SizeBA = BA.Size();
			const float SizeBC = BC.Size();
			float MinVectorSize;

			if (SizeBA > SizeBC)	MinVectorSize = SizeBC;
			else					MinVectorSize = SizeBA;
			
			if (SizeBA == 0.0f || SizeBC == 0.0f || MinVectorSize <= 0 || FMath::IsNearlyZero(FVector2D::CrossProduct(FVector2D(BA.X, BA.Y), FVector2D(BC.X, BC.Y))))
			{
				NewPoints.Add(B);
				continue;
			}
		
			MinVectorSize = (MinVectorSize - MinVectorSize / 100.0f) * 0.5f;
		
			if (Radius < MinVectorSize) MinVectorSize = Radius;
			
			const FVector NewA = BA.GetSafeNormal() * MinVectorSize + B;
			const FVector NewC = BC.GetSafeNormal() * MinVectorSize + B;

			TArray<FVector> BezierPoints = BuildBezierCurve3D(NewA, B, NewC, Segments);
			
			const int NumBezier = BezierPoints.Num();

			for (int j = 0; j < NumBezier; ++j)
			{
				const FVector& NewPoint = BezierPoints[j];
				NewPoints.Add(NewPoint);
			}
		}

		// Add Last Point
		if (bLoopPath)
		{
			const FVector& FirstPoint = NewPoints[0];
			NewPoints.Emplace(FirstPoint);
		}
		else
		{
			const int LastIndex = NumPoints - 1;
			NewPoints.Add(PathPoints[LastIndex]);
		}
	}
	
	// For 2D
	else
	{
		TArray<FVector2D> Points2D;
		TArray<float> PointsZ;
		
		SplitVectorArray(PathPoints, Points2D, PointsZ);

		int Num = NumPoints;

		if (!bLoopPath)
		{
			Num -= 2;
			
			// Add First Point
			NewPoints.Add({PathPoints[0].X, PathPoints[0].Y, PointsZ[0]});
		}
		else
		{
			Num += 0;
		}
		
		// Find Corners Points
		for (int i = 0; i < Num; ++i)
		{
			int PreviousIndex;
			int CurrentIndex;
			int NextIndex;

			if (bLoopPath)
			{
				PreviousIndex = (i + NumPoints - 1) % NumPoints;
				CurrentIndex = i % NumPoints;
				NextIndex = (i + 1) % NumPoints;
			}
			else
			{
				PreviousIndex = i;
				CurrentIndex = (i + 1) % NumPoints;
				NextIndex = (i + 2) % NumPoints;
			}
			
			const FVector2D& A = Points2D[PreviousIndex];
			const FVector2D& B = Points2D[CurrentIndex];
			const FVector2D& C = Points2D[NextIndex];
			
			const FVector2D BA = A - B;
			const FVector2D BC = C - B;

			const float SizeBA = BA.Size();
			const float SizeBC = BC.Size();
			float MinVectorSize;

			if (SizeBA > SizeBC)	MinVectorSize = SizeBC;
			else					MinVectorSize = SizeBA;
			
			if (SizeBA == 0.0f || SizeBC == 0.0f || MinVectorSize <= 0 || FMath::IsNearlyZero(FVector2D::CrossProduct(BA, BC)))
			{
				NewPoints.Add({B.X, B.Y, PointsZ[CurrentIndex]});
				continue;
			}
		
			MinVectorSize = (MinVectorSize - MinVectorSize / 100.0f) * 0.5f;
		
			if (Radius < MinVectorSize) MinVectorSize = Radius;
			
			const FVector2D NewA = BA.GetSafeNormal() * MinVectorSize + B;
			const FVector2D NewC = BC.GetSafeNormal() * MinVectorSize + B;

			TArray<FVector2D> BezierPoints = BuildBezierCurve(NewA, B, NewC, Segments);
			
			const int NumBezier = BezierPoints.Num();

			for (int j = 0; j < NumBezier; ++j)
			{
				const FVector2D& NewPoint = BezierPoints[j];
				NewPoints.Add({NewPoint.X, NewPoint.Y, PointsZ[CurrentIndex]});
			}
		}

		// Add Last Point
		if (!bLoopPath)
		{
			const int LastIndex = NumPoints - 1;
			NewPoints.Add({PathPoints[LastIndex].X, PathPoints[LastIndex].Y, PointsZ[LastIndex]});
		}
		else
		{
			const FVector& FirstPoint = NewPoints[0];
			NewPoints.Emplace(FirstPoint);
		}
	}
	
	return NewPoints;
}

TArray<FVector> ASimplePathTracerActor::LimitPathByDistance(const TArray<FVector>& PathPoints, const float Distance)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || Distance <= 0.0f) return {};

	TArray<FVector> FilteredPoints;

	const int Num = NumPoints - 1;
	
	float DistanceAlongPath = 0.0f;
	
	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];

		const float SegmentDistance = FVector::Dist(CurrentPoint, NextPoint);
		
		const float DistanceRemainder = Distance - DistanceAlongPath;
		
		if (DistanceRemainder > SegmentDistance && i < Num - 1)
		{
			FilteredPoints.Add(CurrentPoint);
			DistanceAlongPath += SegmentDistance;
		}
		else
		{
			// Add Last Point
			FilteredPoints.Add(CurrentPoint);
			
			if (DistanceRemainder >= SegmentDistance)
			{
				// Add Last Path Point
				FilteredPoints.Add(NextPoint);
			}
			else
			{
				FilteredPoints.Add((NextPoint - CurrentPoint).GetSafeNormal() * DistanceRemainder + CurrentPoint);
			}
			
			break;
		}
	}
	
	return FilteredPoints;
}

TArray<FVector> ASimplePathTracerActor::LimitPathByPercent(const TArray<FVector>& PathPoints, const float Percent)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || Percent <= 0.0f)	return {};
	if (Percent >= 1.0f)					return PathPoints;

	float PathLength = GetPathLength(PathPoints);

	PathLength *= Percent;

	TArray<FVector> FilteredPoints;

	const int Num = NumPoints - 1;
	
	float DistanceAlongPath = 0.0f;

	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];

		const float SegmentDistance = FVector::Dist(CurrentPoint, NextPoint);
		
		const float DistanceRemainder = PathLength - DistanceAlongPath;
		
		if (DistanceRemainder > SegmentDistance && i < Num - 1)
		{
			FilteredPoints.Add(CurrentPoint);
			DistanceAlongPath += SegmentDistance;
		}
		else
		{
			// Add Last Point
			FilteredPoints.Add(CurrentPoint);
			
			if (DistanceRemainder >= SegmentDistance)
			{
				FilteredPoints.Add(NextPoint);
			}
			else
			{
				FilteredPoints.Add((NextPoint - CurrentPoint).GetSafeNormal() * DistanceRemainder + CurrentPoint);
			}
			
			break;
		}
	}
	
	return FilteredPoints;
}

float ASimplePathTracerActor::GetPathLength(const TArray<FVector>& PathPoints)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2) return 0.f;

	float Length = 0.f;
	
	const int Num = NumPoints - 1;

	for (int i = 0; i < Num; i++)
	{
		Length += FVector::Dist(PathPoints[i], PathPoints[i + 1]);
	}
	
	return Length;
}

void ASimplePathTracerActor::GetPointAlongPath(const TArray<FVector>& PathPoints, const float Distance, const bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints == 0) return;
	if (NumPoints == 1)
	{
		Location = PathPoints[0];
		return;
	}

	float ClampDistance = FMath::Clamp(Distance, 0.f , Distance);
	
	const int Num = NumPoints - 1;

	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];

		const float SegmentLength = FVector::Dist(CurrentPoint, NextPoint);

		ClampDistance -= SegmentLength;
		
		if (ClampDistance <= 0.0f)
		{
			ClampDistance = FMath::Abs(ClampDistance);
			
			Location = (CurrentPoint - NextPoint).GetSafeNormal() * ClampDistance + NextPoint;

			if (bIn2D)
			{
				Direction = NextPoint - CurrentPoint;
				Direction.Z = 0.0f;
				Direction = Direction.GetSafeNormal();
			}
			else
			{
				Direction = (NextPoint - CurrentPoint).GetSafeNormal();
			}

			Rotation = FRotationMatrix::MakeFromX(-Direction).Rotator();
			
			break;
		}

		if (i == Num - 1)
		{
			Location = NextPoint;

			if(bIn2D)
			{
				Direction = NextPoint - CurrentPoint;
				Direction.Z = 0.0f;
				Direction = Direction.GetSafeNormal();
			}
			else
			{
				Direction = (NextPoint - CurrentPoint).GetSafeNormal();
			}
			
			Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		}
	}
}

void ASimplePathTracerActor::GetPointAlongPathPercent(const TArray<FVector>& PathPoints, const float Percent, const bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints == 0)	return;
	if (NumPoints == 1)
	{
		Location = PathPoints[0];
		return;
	}

	const float ClampPercent = FMath::Clamp(Percent, 0.f , 1.f);

	float PathLength = GetPathLength(PathPoints);

	PathLength *= ClampPercent;
	
	const int Num = NumPoints - 1;

	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];

		const float SegmentLength = FVector::Dist(CurrentPoint, NextPoint);

		PathLength -= SegmentLength;
		
		if (PathLength <= 0.0f)
		{
			PathLength = FMath::Abs(PathLength);
			
			Location = (CurrentPoint - NextPoint).GetSafeNormal() * PathLength + NextPoint;

			if (bIn2D)
			{
				Direction = NextPoint - CurrentPoint;
				Direction.Z = 0.0f;
				Direction = Direction.GetSafeNormal();
			}
			else
			{
				Direction = (NextPoint - CurrentPoint).GetSafeNormal();
			}

			Rotation = FRotationMatrix::MakeFromX(-Direction).Rotator();
			
			break;
		}

		if (i == Num - 1)
		{
			Location = NextPoint;

			if(bIn2D)
			{
				Direction = NextPoint - CurrentPoint;
				Direction.Z = 0.0f;
				Direction = Direction.GetSafeNormal();
			}
			else
			{
				Direction = (NextPoint - CurrentPoint).GetSafeNormal();
			}
			
			Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		}
	}
}

void ASimplePathTracerActor::CalculatePathNormals(const TArray<FVector2D>& PathPoints, TArray<FVector2D>& Normals, TArray<float>& SinAngles)
{
	const int NumPoints = PathPoints.Num();

	if (NumPoints == 0) return;
	
	SinAngles.Reserve(NumPoints);
	Normals.Reserve(NumPoints);
	
	for (int i = 0; i < NumPoints; ++i)
	{
		const FVector2D& Point1 = PathPoints[i];
		const FVector2D& Point2 = PathPoints[(i + 1) % NumPoints];
		const FVector2D& Point3 = PathPoints[(i + 2) % NumPoints];

		const float Angle = FindAngle360(Point1, Point2, Point3);
		const float HalfAngleR = Angle * 0.5;

		const FVector2D V1 = Point1 - Point2;
		
		Normals.Add(RotateVectorRadians(V1, HalfAngleR).GetSafeNormal());

		SinAngles.Add(sin(HalfAngleR));
	}
}

TArray<int> ASimplePathTracerActor::CalculateTriangles(const int NumVertexes, const bool bEnableUV)
{
	int NumQuads;
	TArray<int> Triangles;
	
	if (bEnableUV)		NumQuads = NumVertexes/4;
	else				NumQuads = NumVertexes/2 - 1;
	
	for (int i = 0; i < NumQuads; ++i)
	{
		int j;
		
		if (bEnableUV)	j = i * 4;
		else			j = i * 2;
		
		// Add Tris 1
		Triangles.Add(j);
		Triangles.Add(j + 1);
		Triangles.Add(j + 2);
		
		// Add Tris 2
		Triangles.Add(j + 2);
		Triangles.Add(j + 1);
		Triangles.Add(j + 3);
	}
	
	return Triangles;
}

TArray<FVector2D> ASimplePathTracerActor::CalculateUV(const TArray<FVector2D>& Vertexes,
                                                      const float Thickness,
                                                      const FVector2D ScaleUV,
                                                      const FVector2D OffsetUV,
                                                      const bool bRemoveSeamsOnUV)
{
	const int NumVertexes = Vertexes.Num();
	
	if (NumVertexes < 4 || ScaleUV.X == 0 || ScaleUV.Y == 0) return {};
	
	TArray<FVector2D> UV;

	const int NumUV = NumVertexes / 4;

	const float DoubleThickness = Thickness * 2;

	const FVector2D FinScaleUV = ScaleUV / DoubleThickness;
	
	FVector2D FinOffsetUV = OffsetUV;
	
	float LastOffset = 0.0f;
	
	for (int i = 0; i < NumUV; ++i)
	{
		const int  k = i * 4;
		
		const FVector2D& P0 = Vertexes[k];
		const FVector2D& P1 = Vertexes[k + 1];
		const FVector2D& P2 = Vertexes[k + 2];
		const FVector2D& P3 = Vertexes[k + 3];
		
		// This code arranges the segments so that the middle of the side of one segment
		// coincides with the middle of the side of the next segment
		if(bRemoveSeamsOnUV)
		{
			float LastMidLength;
			
			// Find length of middle line of last segment
			if(i == 0)
			{
				LastMidLength = 0.0f;
			}
			else
			{
				FVector2D LastLeftEdgeMidPoint  = (Vertexes[k - 4] + Vertexes[k - 3]) * 0.5f;
				FVector2D LastRightEdgeMidPoint = (Vertexes[k - 2] + Vertexes[k - 1]) * 0.5f;
			
				LastMidLength = (LastLeftEdgeMidPoint - LastRightEdgeMidPoint).Size();
			}
			
			FVector2D LeftEdgeCurrent = P0 - P1;
			FVector2D TopEdgeCurrent = P0 - P2;
		
			// Projection side Edge of segment to one of parallel edges
			const FVector2D Normal = TopEdgeCurrent.GetSafeNormal();
			const FVector2D Projection = FVector2D::DotProduct(LeftEdgeCurrent, Normal) * Normal;
			const float HalfProjectionLength = Projection.Size() * 0.5f;
			
			// Just for compact UV
			/*if(bCompactUV)
			{
				LastMidLength = LastMidLength - floorf(LastMidLength / DoubleThickness) * DoubleThickness / (ScaleUV.X / ScaleUV.Y);
			}*/
			
			// Checking which point is to the left, Top or Bot
			const float DotProduct = FVector2D::DotProduct(P1 - P0, Normal);
		
			if (DotProduct > 0)
			{
				FinOffsetUV.X = LastMidLength + LastOffset + OffsetUV.X + HalfProjectionLength;
			}
			else
			{
				FinOffsetUV.X = LastMidLength + LastOffset + OffsetUV.X - HalfProjectionLength;
			}
		
			LastOffset += LastMidLength;
		}

		// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
		UV.Append(AlignTrapezoidUV({P0, P1, P2, P3}, FinScaleUV, FinOffsetUV));
	}
	
	return UV;
}

TArray<FVector2D> ASimplePathTracerActor::CalculateHeightUV(const TArray<FVector2D>& Vertexes, const float Height)
{
	TArray<FVector2D> UV;
	
	const int Num = Vertexes.Num() / 4;
	
	float LastOffset = 0.0f;

	for (int i = 0; i < Num; ++i)
	{
		const int j = i * 4;
		const float Lenght = (Vertexes[j] - Vertexes[j + 2]).Size();
		const float U = Lenght / Height;
	
		// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
		UV.Add({LastOffset, 0});
		UV.Add({LastOffset, 1});
		UV.Add({LastOffset + U, 0});
		UV.Add({LastOffset + U, 1});

		LastOffset = (Lenght - Height * floorf(U)) / Height + LastOffset;
	}
	
	return UV;
}


bool ASimplePathTracerActor::FindLineIntersection(const FVector2D& Start1,
                                             const FVector2D& End1,
                                             const FVector2D& Start2,
                                             const FVector2D& End2,
                                             FVector2D& IntersectionPoint)
{
	const FVector2D Dir1 = End1 - Start1;
	const FVector2D Dir2 = End2 - Start2;

	const float Cross = FVector2D::CrossProduct(Dir1, Dir2);

	if (FMath::IsNearlyZero(Cross))
	{
		// Lines are parallel or coincident, there is no point of intersection
		return false;
	}

	const FVector2D Delta = Start2 - Start1;
	const float T1 = FVector2D::CrossProduct(Delta, Dir2) / Cross;
	
	IntersectionPoint = Start1 + T1 * Dir1;

	return true;
}

FVector2D ASimplePathTracerActor::RotateVectorRadians(const FVector2D Vector, const float Angle)
{
	float S, C;
	FMath::SinCos(&S, &C, Angle);
	return FVector2D(
		C * Vector.X - S * Vector.Y,
		S * Vector.X + C * Vector.Y);
}

float ASimplePathTracerActor::FindAngle360(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3)
{
	const FVector2D V1 = P1 - P2;
	const FVector2D V2 = P3 - P2;

	const float Dot = V1.X * V2.X + V1.Y * V2.Y;   
	const float Det = V1.X * V2.Y - V1.Y * V2.X;
	
	return atan2(Det, Dot);
}

TArray<FVector> ASimplePathTracerActor::OffsetPathPoints(const TArray<FVector>& PathPoints, const float Offset)
{
	const int NumPoints = PathPoints.Num();
	const int NumPointsMinusOne = NumPoints - 1;

	if (NumPoints < 2 || FMath::IsNearlyZero(Offset)) return PathPoints;

	TArray<FVector2D> Points2D;
	TArray<float> PointsZ;
	
	TArray<FVector2D> OffsetPoints2D;
	TArray<float> SinAngles;
	TArray<FVector2D> Normals;

	SplitVectorArray(PathPoints, Points2D, PointsZ);

	CalculatePathNormals(Points2D, Normals, SinAngles);
	
	// Offset first point
	FVector2D P1 = Points2D[0];
	
	FVector2D Normal;
	float OffsetLength;
	float SinA = SinAngles[NumPointsMinusOne];
	
	if (FMath::Abs(SinA) < SMALL_NUMBER)
	{
		const FVector2D P2 = Points2D[1];
		const FVector2D V1 = P2 - P1;
		const float Length = V1.SizeSquared();

		if(Length < SMALL_NUMBER)
		{
			Normal = (P2 - P1).GetSafeNormal();
			Normal = {-Normal.Y, Normal.X};
		}
		else
		{
			Normal = (P2 - P1).GetSafeNormal();
			Normal = {-Normal.Y, Normal.X};
		}

		OffsetLength = Offset;
	}
	else
	{
		Normal = Normals[NumPointsMinusOne];
		OffsetLength = Offset / SinA;
	}
	
	OffsetPoints2D.Add(P1 + Normal * OffsetLength);

	
	// Offset other points
	for (int i = 0; i < NumPointsMinusOne; ++i)
	{
		P1 = Points2D[i + 1];
		
		Normal = Normals[i];
		SinA = SinAngles[i];

		if (FMath::Abs(SinA) < SMALL_NUMBER)
		{
			OffsetLength = Offset;
		}
		else
		{
			OffsetLength = Offset / SinA;
		}
		
		OffsetPoints2D.Add(P1 + Normal * OffsetLength);
	}

	TArray<FVector> OffsetPoints;
	
	CombineVectorArray(OffsetPoints2D, PointsZ, OffsetPoints);
	
	return OffsetPoints;
}

void ASimplePathTracerActor::SplitVectorArray(const TArray<FVector>& Points3D, TArray<FVector2D>& Points2D, TArray<float>& PointsZ)
{
	const int NumPoints = Points3D.Num();

	if (NumPoints == 0) return;

	Points2D.Reserve(NumPoints);
	PointsZ.Reserve(NumPoints);
	
	for (const auto& Point : Points3D)
	{
		Points2D.Emplace(FVector2D(Point.X, Point.Y));
		PointsZ.Add(Point.Z);
	}
}

void ASimplePathTracerActor::CombineVectorArray(const TArray<FVector2D>& Points2D, const TArray<float>& PointsZ, TArray<FVector>& Points3D)
{
	const int NumPoints = Points2D.Num();
	const int NumPointsZ = PointsZ.Num();

	if (NumPoints == 0 || NumPoints != NumPointsZ) return;

	Points3D.Reserve(NumPoints);

	for (int i = 0; i < NumPoints; ++i)
	{
		const FVector2D& Point = Points2D[i];
		Points3D.Emplace(FVector(Point.X, Point.Y, PointsZ[i]));
	}
}

TArray<FVector> ASimplePathTracerActor::GetPointsAlongPath(const TArray<FVector>& PathPoints, const float Interval, const bool bAddLastPoint)
{
	const int NumPoints = PathPoints.Num();

	if(NumPoints == 0 || Interval <= 0.01f) return {};
	if(NumPoints == 1) return {PathPoints[0]};

	TArray<FVector> PointsAlongPath;

	float CurrentDistance = 0.0f;

	// Add first Point
	PointsAlongPath.Add(PathPoints[0]);

	// Add Middle Points
	for (int i = 1; i < NumPoints; ++i)
	{
		const FVector& P1 = PathPoints[i];
		const FVector& P2 = PathPoints[i - 1];
		
		const float SegmentLength = FVector::Dist(P1, P2);

		while (CurrentDistance + Interval < SegmentLength)
		{
			const FVector Direction = (P1 - P2).GetSafeNormal();
			const FVector NewPoint = P2 + Direction * (CurrentDistance + Interval);
			PointsAlongPath.Add(NewPoint);
			CurrentDistance += Interval;
		}

		CurrentDistance -= SegmentLength;
	}

	// Add last Point
	if (bAddLastPoint)
	{
		PointsAlongPath.Add(PathPoints[NumPoints - 1]);
	}
	
	return PointsAlongPath;
}

void ASimplePathTracerActor::GetPointsAlongPathDirected(const TArray<FVector>& PathPoints, TArray<FVector>& Points, TArray<FVector>& Directions,
                                                   const float Interval, const bool bAddLastPoint)
{
	const int NumPoints = PathPoints.Num();

	if(NumPoints == 0 || Interval <= 0.01f) return;
	if(NumPoints == 1)
	{
		Points.Add(PathPoints[0]);
		Directions.Add({1,0,0});
		return;
	}

	float CurrentDistance = 0.0f;

	// Add first Point
	Points.Add(PathPoints[0]);
	Directions.Add((PathPoints[1] - PathPoints[0]).GetSafeNormal());
	
	// Add Middle Points
	for (int i = 1; i < NumPoints; ++i)
	{
		const FVector& P1 = PathPoints[i];
		const FVector& P2 = PathPoints[i - 1];
		
		const float SegmentLength = FVector::Dist(P1, P2);

		while (CurrentDistance + Interval < SegmentLength)
		{
			const FVector Direction = (P1 - P2).GetSafeNormal();
			const FVector NewPoint = P2 + Direction * (CurrentDistance + Interval);
			Points.Add(NewPoint);
			Directions.Add(Direction);
			CurrentDistance += Interval;
		}

		CurrentDistance -= SegmentLength;
	}

	// Add last Point
	if (bAddLastPoint)
	{
		Points.Add(PathPoints[NumPoints - 1]);
		Directions.Add((PathPoints[NumPoints - 1] - PathPoints[NumPoints - 2]).GetSafeNormal());
	}
}

TArray<FVector> ASimplePathTracerActor::FixVerticalPathPoints(const TArray<FVector>& PathPoints, const float Offset, const bool bFixOverlap, const float FixTolerance)
{
	int NumPoints = PathPoints.Num();
	
	if (NumPoints < 3 || Offset <= 0.0f) return PathPoints;

	TArray<FVector> OffsetPoints = PathPoints;

	for (int i = 1; i < NumPoints - 1; ++i)
	{
		const FVector& P1 = OffsetPoints[i - 1];
		const FVector& P2 = OffsetPoints[i];
		const FVector& P3 = OffsetPoints[i + 1];

		if (bFixOverlap)
		{
			FVector2D V1 = {P2.X - P1.X, P2.Y - P1.Y};
			FVector2D V2 = {P3.X - P2.X, P3.Y - P2.Y};

			const float DotProduct = FVector2D::DotProduct(V1.GetSafeNormal(), V2.GetSafeNormal());

			// If angle between V1 and V2 = 180
			if(FMath::IsNearlyEqual(DotProduct, -1.0f, FixTolerance))
			{
				OffsetPoints[i + 1].X = P2.X;
				OffsetPoints[i + 1].Y = P2.Y;
			}
		}
		
		if (FMath::IsNearlyEqual(P1.X, P2.X) && FMath::IsNearlyEqual(P1.Y, P2.Y))
		{
			if(FMath::IsNearlyEqual(P1.Z, P2.Z))
			{
				OffsetPoints.RemoveAt(i);
				--NumPoints;
			}
			else
			{
				const FVector PointOffset = (P3 - P2).GetSafeNormal() * Offset + P2;
				OffsetPoints[i] = PointOffset;
			}
		}
	}
	
	return OffsetPoints;
}

TArray<FVector> ASimplePathTracerActor::AlignPathPointsInHeight(const TArray<FVector>& PathPoints, const float Height)
{
	TArray<FVector> AlignPoints;
	
	for (const auto& Point : PathPoints)
	{
		AlignPoints.Add({Point.X, Point.Y, Height});
	}

	return AlignPoints;
}

TArray<FVector> ASimplePathTracerActor::RemovePathPointsLyingOnLine(const TArray<FVector>& PathPoints, const float Tolerance)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 3) return PathPoints;

	TArray<FVector> FilteredPoints;

	FilteredPoints.Add(PathPoints[0]);
	
	for (int i = 1; i < NumPoints - 1; ++i)
	{
		const FVector& P1 = FilteredPoints[FilteredPoints.Num() - 1];
		const FVector& P2 = PathPoints[i];
		const FVector& P3 = PathPoints[i + 1];
		
		const FVector V1 = P2 - P1;
		const FVector V2 = P3 - P2;
		
		const FVector Cross = FVector::CrossProduct(V1, V2);
		
		if (FMath::IsNearlyZero(Cross.X, Tolerance)
			&& FMath::IsNearlyZero(Cross.Y, Tolerance)
			&& FMath::IsNearlyZero(Cross.Z, Tolerance))
		{
			
		}
		else
		{
			FilteredPoints.Add(P2);
		}
	}

	FilteredPoints.Add(PathPoints[NumPoints - 1]);
	
	return FilteredPoints;
}

TArray<FVector> ASimplePathTracerActor::AddPathCornersOffset(const TArray<FVector>& PathPoints, const float Offset)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2) return PathPoints;

	TArray<FVector> ModifiedPoints;

	for (int i = 0; i < NumPoints - 1; i++) {
		const FVector& CurrentVertex = PathPoints[i];
		const FVector& NextVertex = PathPoints[i + 1];

		const FVector V1 = NextVertex - CurrentVertex;
		const FVector Normal = V1.GetSafeNormal();
		
		const float SegmentLength = V1.Size();

		const FVector P1 = Normal * Offset + CurrentVertex;
		const FVector P2 = -Normal * Offset + NextVertex;
		
		if ((CurrentVertex - P1).Size() < SegmentLength / 2 - SegmentLength * 0.01f)
		{	
			ModifiedPoints.Add(P1);
			ModifiedPoints.Add(P2);
		}
	}

	return ModifiedPoints;
}

void ASimplePathTracerActor::CombinePathsData(const TArray<FVector>& Vertexes1, const TArray<FVector>& Vertexes2,
                                          const TArray<int>& Triangles1, const TArray<int>& Triangles2,
                                          const TArray<FVector2D>& UV1,
                                          const TArray<FVector2D>& UV2,
                                          TArray<FVector>& Vertexes, TArray<int>& Triangles, TArray<FVector2D>& UV)
{
	Vertexes.Append(Vertexes1);
	Vertexes.Append(Vertexes2);

	Triangles.Append(Triangles1);

	const int NumTriangles = Triangles2.Num();
	const int NumVertexes = Vertexes2.Num();

	for (int i = 0; i < NumTriangles; ++i)
	{
		Triangles.Add(Triangles2[i] + NumVertexes);
	}
	
	UV.Append(UV1);
	UV.Append(UV2);
}

TArray<FVector> ASimplePathTracerActor::TrimPathFromStart(const TArray<FVector>& PathPoints, const float TrimLength)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || TrimLength <= 0.0f) return PathPoints;

	TArray<FVector> TrimPoints;
	int RemoveCount = 0;
	FVector LastPoint = {};

	const int Num = NumPoints - 1;
	
	float TrimRemainder = TrimLength;
	
	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];

		const float SegmentDistance = FVector::Dist(CurrentPoint, NextPoint);
		
		if (TrimRemainder > SegmentDistance)
		{
			if (i != Num - 1)
			{
				++RemoveCount;
				TrimRemainder -= SegmentDistance;
			}
			else
			{
				RemoveCount = NumPoints;
				break;
			}
		}
		else
		{
			++RemoveCount;
			LastPoint = (NextPoint - CurrentPoint).GetSafeNormal() * TrimRemainder + CurrentPoint;
			break;
		}
	}

	if (RemoveCount < NumPoints)
	{
		TrimPoints.Emplace(LastPoint);
		
		for (int i = RemoveCount; i < NumPoints; ++i)
		{
			TrimPoints.Emplace(PathPoints[i]);
		}
	}
	else
	{
		TrimPoints.Emplace(PathPoints[Num]);
	}
	
	return TrimPoints;
}

TArray<FVector> ASimplePathTracerActor::TrimPathFromEnd(const TArray<FVector>& PathPoints, const float TrimLength)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2 || TrimLength <= 0.0f) return PathPoints;

	TArray<FVector> TrimPoints;
	int RemoveCount = 0;
	FVector LastPoint = {};

	const int Num = NumPoints - 1;
	
	float TrimRemainder = TrimLength;
	
	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[(Num - i) % NumPoints];
		const FVector& NextPoint	= PathPoints[(Num - i - 1) % NumPoints];

		const float SegmentDistance = FVector::Dist(CurrentPoint, NextPoint);
		
		if (TrimRemainder > SegmentDistance)
		{
			if (i != Num - 1)
			{
				++RemoveCount;
				TrimRemainder -= SegmentDistance;
			}
			else
			{
				RemoveCount = NumPoints;
				break;
			}
		}
		else
		{
			++RemoveCount;
			LastPoint = (NextPoint - CurrentPoint).GetSafeNormal() * TrimRemainder + CurrentPoint;
			break;
		}
	}

	if (RemoveCount < NumPoints)
	{
		const int NumRem = NumPoints - RemoveCount;
		
		for (int i = 0; i < NumRem; ++i)
		{
			TrimPoints.Emplace(PathPoints[i]);
		}

		TrimPoints.Emplace(LastPoint);
	}
	else
	{
		TrimPoints.Emplace(PathPoints[0]);
	}
	
	return TrimPoints;
}

TArray<FVector> ASimplePathTracerActor::RemoveSmallSegmentsCloseToEnd(const TArray<FVector>& PathPoints, const float Length, const bool bFromStart)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 3) return PathPoints;

	TArray<FVector> RemovePoints = PathPoints;

	int RemoveIndex = NumPoints - 2;
	int LastIndex = NumPoints - 1;
	
	if (bFromStart)
	{
		RemoveIndex = 1;
		LastIndex = 0;
	}

	const FVector& LastPoint = RemovePoints[LastIndex];

	for (int i = 1; i < NumPoints; ++i)
	{
		const float SegmentLength = FVector::Distance(LastPoint, RemovePoints[RemoveIndex]);
		
		if (SegmentLength < Length)
		{
			RemovePoints.RemoveAt(RemoveIndex);
			if (!bFromStart) --RemoveIndex;
		}
		else
		{
			break;
		}
	}
		
	return RemovePoints;
}

TArray<FVector> ASimplePathTracerActor::RemoveSmallSegmentsCloseToBothEnds(const TArray<FVector>& PathPoints,
																		   const bool bEnableStart, const float StartLength,
																		   const bool bEnableEnd, const float EndLength)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 3) return PathPoints;

	const FVector& EndPoint = PathPoints[NumPoints - 1];

	if (bEnableEnd)
	{
		const TArray<FVector> EndPoints = RemoveSmallSegmentsCloseToEnd(PathPoints, EndLength, false);

		if (bEnableStart)
		{
			const TArray<FVector> StartPoints = RemoveSmallSegmentsCloseToEnd(EndPoints, StartLength, true);

			if (StartPoints.Num() > 1) return StartPoints;
			
			return {EndPoint};
		} 

		return EndPoints;
	}

	if (bEnableStart)
	{
		const TArray<FVector> StartPoints = RemoveSmallSegmentsCloseToEnd(PathPoints, StartLength, true);
		
		if (StartPoints.Num() > 1) return StartPoints;
			
		return {EndPoint};
	}
	
	return PathPoints;
}

void ASimplePathTracerActor::TrimPathComplex(const TArray<FVector>& PathPoints,
                                             TArray<FVector>& TrimPoints,
                                             FVector& StartTrimPoint,
                                             FRotator& StartRotation,
                                             FVector& StartDirection,
                                             FVector& EndTrimPoint,
                                             FRotator& EndRotation,
                                             FVector& EndDirection,
                                             const bool bEnableStartTrim,
                                             const float StartTrim,
                                             const bool bEnableEndTrim,
                                             const float EndTrim)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints == 0) return;
	if (NumPoints == 1)
	{
		const FVector& P = PathPoints[0];
		
		if (bEnableStartTrim)
		{
			if (bEnableEndTrim) StartTrimPoint = P + FVector( EndTrim, 0.0f, 0.0f);
			else StartTrimPoint = P + FVector( -StartTrim, 0.0f, 0.0f);
		} 
		else StartTrimPoint = P;
		StartDirection = {1.0f, 0.0f, 0.0f};
		
		if (bEnableEndTrim) EndTrimPoint = P + FVector(EndTrim, 0.0f, 0.0f);
		else EndTrimPoint = P;
		EndDirection = {1.0f, 0.0f, 0.0f};
		EndRotation = {1.0f, 180.0f, 0.0f};
		
		TrimPoints.Emplace(P);
		
		return;
	}

	if (bEnableStartTrim)
	{
		TrimPoints = TrimPathFromStart(PathPoints, StartTrim);
		
		if (bEnableEndTrim && TrimPoints.Num() > 1)
		{
			TrimPoints = TrimPathFromEnd(TrimPoints, EndTrim);
		}
	}
	else
	{
		if (bEnableEndTrim)
		{
			TrimPoints = TrimPathFromEnd(PathPoints, EndTrim);

			if (TrimPoints.Num() == 1)
			{
				TrimPoints[0] = PathPoints[NumPoints - 1];
			}
		}
		else
		{
			TrimPoints = PathPoints;
		}
	}

	const int NumTrimPoints = TrimPoints.Num();

	if (NumTrimPoints == 0) return;

	const FVector& P1 = PathPoints[0];
	const FVector& P2 = PathPoints[1];
	const FVector& P3 = PathPoints[NumPoints - 2];
	const FVector& P4 = PathPoints[NumPoints - 1];

	const FVector V1 = P2 - P1;
	StartDirection = V1.GetSafeNormal();
	StartTrimPoint = TrimPoints[0];
	StartRotation = FRotationMatrix::MakeFromX(-StartDirection).Rotator();
	
	const FVector V2 = P4 - P3;
	EndDirection = V2.GetSafeNormal();
	EndTrimPoint = TrimPoints[NumTrimPoints - 1];
	EndRotation = FRotationMatrix::MakeFromX(EndDirection).Rotator();
}

void ASimplePathTracerActor::GetQuadData(TArray<FVector>& Vertexes,
                                         TArray<int>& Triangles,
                                         TArray<FVector2D>& UV,
                                         const FVector Center,
                                         const FVector Direction,
                                         const float Length,
                                         const float Width,
                                         const bool bEnableUV)
{
	const FVector RightVector = FVector::CrossProduct(Direction, {0,0,1}).GetSafeNormal();
	
	const FVector L = Direction * Length * 0.5f;
	const FVector W = -RightVector * Width * 0.5f;

	const FVector LW = L + W;
	const FVector LWm = L - W;

	const int NumVerts = Vertexes.Num();
	
	Vertexes.Add(-LW + Center);
	Vertexes.Add(LWm + Center);
	Vertexes.Add(-LWm + Center);
	Vertexes.Add(LW + Center);
	
	Triangles.Add(NumVerts);
	Triangles.Add(NumVerts + 2);
	Triangles.Add(NumVerts + 1);
	
	Triangles.Add(NumVerts + 2);
	Triangles.Add(NumVerts + 3);
	Triangles.Add(NumVerts + 1);

	if (!bEnableUV) return;
	
	// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
	UV.Add({0, 0});
	UV.Add({0, 1});
	UV.Add({1, 0});
	UV.Add({1, 1});
}

FVector2D ASimplePathTracerActor::FindCornerMidDirection(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3)
{
	const FVector2D V1 = P1 - P2;
	const FVector2D V2 = P3 - P2;

	const float Dot = V1.X * V2.X + V1.Y * V2.Y;
	const float Det = V1.X * V2.Y - V1.Y * V2.X;

	return RotateVectorRadians(V1, atan2(Det, Dot) * 0.5f).GetSafeNormal();
}

TArray<FLinearColor> ASimplePathTracerActor::GetVertexColorData(const TArray<FVector>& PathPoints,
                                                                const bool bEnableStartOpacity,
                                                                const float StartDistance,
                                                                const bool bEnableEndOpacity,
                                                                const float EndDistance,
                                                                const bool bEnableUV,
                                                                const UCurveLinearColor* ColorCurve,
                                                                const bool bUnitCurve,
                                                                const float Hardness,
                                                                const FLinearColor DefaultColor)
{
	const int NumPoints = PathPoints.Num();

	if (NumPoints < 2) return {};

	if (NumPoints == 2)
	{
		FLinearColor StartColor = DefaultColor;
		FLinearColor EndColor   = DefaultColor;
		if (ColorCurve)
		{
			StartColor = ColorCurve->GetLinearColorValue(0.f);
			EndColor   = ColorCurve->GetLinearColorValue(1.f);
		}
		if (bEnableStartOpacity) StartColor.A = 0.5f;
		if (bEnableEndOpacity)   EndColor.A   = 0.5f;
		
		return {StartColor, StartColor, EndColor, EndColor};
	} 
	
	if (!(bEnableStartOpacity || bEnableEndOpacity || ColorCurve)) return {};

	int NumVertexes;
	if (bEnableUV) NumVertexes = NumPoints * 4 - 4;
	else		   NumVertexes = NumPoints * 2;
	
	TArray<FLinearColor> VertexColors;
	TArray<float> StartWeights;
	TArray<float> EndWeights;
	
	VertexColors.Init(DefaultColor, NumVertexes);
	StartWeights.Init(1.f, NumPoints);
	EndWeights.Init(1.f, NumPoints);

	const float NumMinusOne = NumPoints - 1;
	const float K = 1 - exp(-Hardness);
	
	// Set Color
	if (ColorCurve)
	{
		const float PathLength = GetPathLength(PathPoints);

		FLinearColor Color = ColorCurve->GetLinearColorValue(0.f);

		VertexColors[0] = Color;
		VertexColors[1] = Color;

		float CurrentDistance = 0.f;
		
		for (int i = 1; i < NumMinusOne; ++i)
		{
			int j;
			float Time;

			if (bUnitCurve) Time = CurrentDistance / PathLength;
			else			Time = CurrentDistance;

			Color = ColorCurve->GetLinearColorValue(Time);

			if (bEnableUV)
			{
				j = i * 4 - 2;
				
				VertexColors[j]     = Color;
				VertexColors[j + 1] = Color;
				VertexColors[j + 2] = Color;
				VertexColors[j + 3] = Color;
			}
			else
			{
				j = i * 2;
				
				VertexColors[j]     = Color;
				VertexColors[j + 1] = Color;
			}

			CurrentDistance += FVector::Dist(PathPoints[i], PathPoints[i + 1]);
		}

		if (bUnitCurve)	Color = ColorCurve->GetLinearColorValue(1.f);
		else 			Color = ColorCurve->GetLinearColorValue(PathLength);
	

		VertexColors[NumVertexes - 2] = Color;
		VertexColors[NumVertexes - 1] = Color;
	}
	
	// Start Alpha
	if (bEnableStartOpacity)
	{
		StartWeights[0] = 0.f;
		
		float CurrentDistance = 0.f;
		
		for (int i = 0; i < NumPoints; i++)
		{
			if (CurrentDistance >= StartDistance) break;

			StartWeights[i] = CurrentDistance / StartDistance * K;
			
			if (i != NumMinusOne)
			{
				CurrentDistance += FVector::Dist(PathPoints[i], PathPoints[i + 1]);
			}
		}
	}

	// End Alpha
	if (bEnableEndOpacity)
	{
		EndWeights[NumMinusOne] = 0.f;
		
		float CurrentDistance = 0.f;
		
		for (int i = 0; i < NumPoints; i++)
		{
			if (CurrentDistance >= EndDistance) break;

			EndWeights[NumMinusOne - i] = CurrentDistance / EndDistance * K;
			
			if (i != NumMinusOne)
			{
				CurrentDistance += FVector::Dist(PathPoints[NumMinusOne - i - 1], PathPoints[NumMinusOne - i]);
			}
		}
	}
	
	// Set Vertexes Alpha
	for (int i = 0; i < NumPoints; i++)
	{
		float Weight;
		
		if (bEnableStartOpacity && bEnableEndOpacity) Weight = FMath::Min(StartWeights[i], EndWeights[i]);
		else if (bEnableStartOpacity)			Weight = StartWeights[i];
		else							Weight = EndWeights[i];
		
		int  j;
		
		if (bEnableUV && i != 0)
		{
			// Two last vertex Alpha
			if (i == NumMinusOne)
			{
				VertexColors[NumVertexes - 2].A = Weight;
				VertexColors[NumVertexes - 1].A = Weight;
				break;
			}
			
			j = i * 4 - 2;
			
			// Double Vertexes
			VertexColors[j].A     = Weight;
			VertexColors[j + 1].A = Weight;
			VertexColors[j + 2].A = Weight;
			VertexColors[j + 3].A = Weight;
		}
		else
		{
			j = i * 2;
			
			VertexColors[j].A     = Weight;
			VertexColors[j + 1].A = Weight;
		}
	}
	
	return VertexColors;
}

void ASimplePathTracerActor::SplitPath(const TArray<FVector> PathPoints, const float Distance, TArray<FVector>& StartPart, TArray<FVector>& EndPart)
{
	const int NumPoints = PathPoints.Num();
	
	if (NumPoints < 2) return;
	if (Distance <= 0.0f)
	{
		EndPart = PathPoints;
		return;
	} 
	
	const int Num = NumPoints - 1;
	float LocDistance = Distance;

	for (int i = 0; i < Num; i++)
	{
		const FVector& CurrentPoint = PathPoints[i];
		const FVector& NextPoint	= PathPoints[i + 1];
		
		StartPart.Emplace(CurrentPoint);

		const float SegmentLength = FVector::Dist(CurrentPoint, NextPoint);

		LocDistance -= SegmentLength;
		
		if (LocDistance < 0.0f)
		{
			LocDistance = FMath::Abs(LocDistance);
			
			FVector SplitPoint = (CurrentPoint - NextPoint).GetSafeNormal() * LocDistance + NextPoint;

			StartPart.Emplace(SplitPoint);
			EndPart.Emplace(SplitPoint);
			
			for (int j = i + 1; j <= Num; ++j)
			{
				EndPart.Emplace(PathPoints[j]);
			}
			
			break;
		}
		
		if (FMath::IsNearlyZero(LocDistance))
		{
			FVector SplitPoint = NextPoint;

			StartPart.Emplace(SplitPoint);
			
			for (int j = i + 1; j <= Num; ++j)
			{
				EndPart.Emplace(PathPoints[j]);
			}
			
			break;
		}

		if (i == Num - 1)
		{
			StartPart.Emplace(NextPoint);
		}
	}
}

bool ASimplePathTracerActor::CheckIfPointLiesInsideArea(const TArray<FVector> AreaPoints, const FVector& Point)
{
	const int NumPoints = AreaPoints.Num();
	
	if (NumPoints < 3) return false;

	// Last Point
	FVector LP = AreaPoints[NumPoints - 1];

	bool bResult = false;

	for (int i = 0; i < NumPoints - 1; ++i)
	{
		// Current Point
		const FVector& CP = AreaPoints[i];
		
		if ((((CP.Y < Point.Y) && (LP.Y >= Point.Y)) || ((LP.Y < Point.Y) && (CP.Y >= Point.Y))) && ((CP.X + (((Point.Y - CP.Y) / (LP.Y - CP.Y)) * (LP.X - CP.X))) < Point.X))
		{
			bResult = !bResult;
		}

		LP = CP;
	}
	
	return bResult;
}

TArray<FVector> ASimplePathTracerActor::MergePathPoints(const TArray<FVector>& PathPoints, const float MergedDistance)
{
	const int NumPoints = PathPoints.Num();

	if(NumPoints < 2 || MergedDistance < 0.0f || FMath::IsNearlyZero(MergedDistance)) return PathPoints;
		
	TArray<FVector> MergedPoints;

	MergedPoints.Add(PathPoints[0]);

	for (int i = 1; i < NumPoints; ++i)
	{
		FVector CurrentPoint = MergedPoints.Last();
		FVector NextPoint = PathPoints[i];

		const float Distance = FVector::Distance(CurrentPoint, NextPoint);
		
		if (Distance > MergedDistance)
		{
			MergedPoints.Add(NextPoint);
		}
		else
		{
			const int LastIndex = MergedPoints.Num() - 1;
			MergedPoints[LastIndex] = (CurrentPoint + NextPoint) * 0.5f;
		}
	}
	
	return MergedPoints;
}

void ASimplePathTracerActor::FindVertexes(const TArray<FVector2D>& Points2D,
                                     const TArray<float>& PointsZ,
                                     const TArray<float>& SinAngles,
                                     const TArray<FVector2D>& Normals,
                                     const bool bEnableUV,
                                     const float Thickness,
                                     const float Offset,
                                     TArray<FVector2D>& Vertexes, TArray<float>& VertexesZ)
{
	// Add to array two first edge points
	const int NumPoints = Points2D.Num();
	const int NumPointsMinusTwo = NumPoints - 2;
	
	const FVector2D StartPoint = Points2D[0];
	const float StartHeight = PointsZ[0];
	const FVector2D FirstSegment = StartPoint - Points2D[1];

	FVector2D Normal = FVector2D(-FirstSegment.Y, FirstSegment.X).GetSafeNormal();

	float CurrentThicknessIn  = Thickness + Offset;
	float CurrentThicknessOut = Thickness - Offset;
	
	Vertexes.Add(StartPoint + Normal * CurrentThicknessIn);
	Vertexes.Add(StartPoint - Normal * CurrentThicknessOut);

	VertexesZ.Add(StartHeight);
	VertexesZ.Add(StartHeight);

	
	// Add to array middle edges points
	for (int i = 0; i < NumPointsMinusTwo; ++i)
	{
		const FVector2D P2 = Points2D[i + 1];
		const float Height = PointsZ[i + 1];
		
		Normal = Normals[i];
		const float SinA = SinAngles[i];
		
		CurrentThicknessIn  = (Thickness + Offset) / SinA;
		CurrentThicknessOut = (Thickness - Offset) / SinA;
		
		FVector2D PointIn  = P2 + Normal * CurrentThicknessIn;
		FVector2D PointOut = P2 - Normal * CurrentThicknessOut;
	
		if (bEnableUV)
		{
			// Duplicate Vertexes for uniq UV
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);

			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
		}
		else
		{
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);

			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
		}
	}

	
	// Add to array two last edge points
	const FVector2D EndPoint = Points2D[NumPoints - 1];
	const float EndHeight = PointsZ[NumPoints - 1];
	const FVector2D LastSegment = Points2D[NumPoints - 2] - EndPoint;
	
	Normal = FVector2D(-LastSegment.Y, LastSegment.X).GetSafeNormal();
	
	CurrentThicknessIn  = Thickness + Offset;
	CurrentThicknessOut = Thickness - Offset;
	
	Vertexes.Add(EndPoint + Normal * CurrentThicknessIn);
	Vertexes.Add(EndPoint - Normal * CurrentThicknessOut);

	VertexesZ.Add(EndHeight);
	VertexesZ.Add(EndHeight);
}

void ASimplePathTracerActor::FindVerticalVertexes(const TArray<FVector>& Points,
                                             const bool bLoopPath,
                                             const bool bEnableUV,
                                             const float Height,
                                             const float Offset,
                                             TArray<FVector>& Vertexes)
{
	const int NumPoints = Points.Num();

	int Num = NumPoints;
		
	if (bLoopPath)
	{
		++Num;
	}
	
	for (int i = 0; i < Num; ++i)
	{
		const FVector& P = Points[i % NumPoints];
		const FVector P1 = {P.X, P.Y, P.Z + Offset};
		const FVector P2 = {P.X, P.Y, P.Z + Height + Offset};
	
		if (bEnableUV && i != 0 && i != Num - 1)
		{
			// Duplicate Vertexes for uniq UV
			Vertexes.Add(P1);
			Vertexes.Add(P2);
			Vertexes.Add(P1);
			Vertexes.Add(P2);
		}
		else
		{
			Vertexes.Add(P1);
			Vertexes.Add(P2);
		}
	}
}

void ASimplePathTracerActor::FindVertexesLoop(const TArray<FVector2D>& Points2D,
                                         const TArray<float>& PointsZ,
                                         const TArray<float>& SinAngles,
                                         const TArray<FVector2D>& Normals,
                                         const bool bEnableUV,
                                         const float Thickness,
                                         const float Offset,
                                         TArray<FVector2D>& Vertexes, TArray<float>& VertexesZ)
{
	const int NumPoints = Points2D.Num();
	
	for (int i = 0; i < NumPoints; ++i)
	{
		const int NextPoint = (i + 1) % NumPoints;
		
		const FVector2D Point2 = Points2D[NextPoint];
		const float Height = PointsZ[NextPoint];
		
		FVector2D Normal;

		Normal = Normals[i];
		const float SinA = SinAngles[i];
		
		const float CurrentThicknessIn  = (Thickness + Offset) / SinA;
		const float CurrentThicknessOut = (Thickness - Offset) / SinA;
		
		FVector2D PointIn  = Point2 + Normal * CurrentThicknessIn;
		FVector2D PointOut = Point2 - Normal * CurrentThicknessOut;

		//Skip first if UV Enabled
		if (bEnableUV && i != 0)
		{
			// Duplicate Vertexes for uniq UV
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);

			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
		}
		else
		{
			Vertexes.Add(PointIn);
			Vertexes.Add(PointOut);

			VertexesZ.Add(Height);
			VertexesZ.Add(Height);
		}
	}

	// Add the first 2 points to the end to make it easier to loop through the points
	if (Vertexes.Num() > 1)
	{
		FVector2D Point = Vertexes[0];
		float Height = VertexesZ[0];
		Vertexes.Add(Point);
		VertexesZ.Add(Height);
		
		Point = Vertexes[1];
		Height = VertexesZ[1];
		Vertexes.Add(Point);
		VertexesZ.Add(Height);
	}
}

void ASimplePathTracerActor::CalculateQuad(const FVector& CenterLocation,
                                           const FVector& Direction,
                                           const float Length,
                                           const float Width,
                                           const bool bEnableUV,
                                           const FVector2D& ScaleUV,
                                           const int DirectionUV,
                                           const FVector2D& OffsetUV,
                                           TArray<FVector>& Vertexes,
                                           TArray<int>& Triangles,
                                           TArray<FVector2D>& UV)
{
	const FVector RightVector = FVector::CrossProduct(Direction, {0,0,1}).GetSafeNormal();
	
	const FVector L = Direction * Length * 0.5f;
	const FVector W = -RightVector * Width * 0.5f;

	const FVector LW = L + W;
	const FVector LWm = L - W;

	const int NumVerts = Vertexes.Num();
	
	Vertexes.Add(-LW + CenterLocation);
	Vertexes.Add(LWm + CenterLocation);
	Vertexes.Add(-LWm + CenterLocation);
	Vertexes.Add(LW + CenterLocation);
	
	Triangles.Add(NumVerts);
	Triangles.Add(NumVerts + 2);
	Triangles.Add(NumVerts + 1);
	
	Triangles.Add(NumVerts + 2);
	Triangles.Add(NumVerts + 3);
	Triangles.Add(NumVerts + 1);

	if (!bEnableUV) return;

	//const float Scale = Length / Width;
	const float U = ScaleUV.X + OffsetUV.X;
	const float V = ScaleUV.Y + OffsetUV.Y;

	const int Index = abs(DirectionUV) % 4;

	// UV Rotation
	switch (Index)
	{
	case 0:
		UV.Add({OffsetUV.X, OffsetUV.Y});
		UV.Add({U, OffsetUV.Y});
		UV.Add({OffsetUV.X, V});
		UV.Add({U, V});
		break;
		
	case 1:
		// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
		UV.Add({OffsetUV.X, OffsetUV.Y});
		UV.Add({OffsetUV.X, V});
		UV.Add({U, OffsetUV.Y});
		UV.Add({U, V});
		break;
		
	case 2:
		UV.Add({U, OffsetUV.Y});
		UV.Add({OffsetUV.X, OffsetUV.Y});
		UV.Add({U, V});
		UV.Add({OffsetUV.X, V});
		break;
		
	case 3:
		UV.Add({U, OffsetUV.Y});
		UV.Add({U, V});
		UV.Add({OffsetUV.X, OffsetUV.Y});
		UV.Add({OffsetUV.X, V});
		break;
			
	default: ;
	}
}

FVector2D ASimplePathTracerActor::CalculateBezierPoint(const float T, const FVector2D& P0, const FVector2D& P1, const FVector2D& P2)
{
	const float U = 1.0f - T;
	const float TT = T * T;
	const float UU = U * U;

	FVector2D P;
	
	P.X = UU * P0.X + 2 * U * T * P1.X + TT * P2.X;
	P.Y = UU * P0.Y + 2 * U * T * P1.Y + TT * P2.Y;
	
	return P;
}

TArray<FVector2D> ASimplePathTracerActor::BuildBezierCurve(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const int NumPoints)
{
	TArray<FVector2D> CurvePoints;
	
	if (NumPoints <= 0) return CurvePoints;
	
	for (int i = 0; i < NumPoints; i++)
	{
		const float T = static_cast<float>(i) / static_cast<float>(NumPoints);
		FVector2D Point = CalculateBezierPoint(T, P0, P1, P2);
		CurvePoints.Add(Point);
	}

	CurvePoints.Add(P2);
	
	return CurvePoints;
}

TArray<FVector2D> ASimplePathTracerActor::AlignTrapezoidUV(const TArray<FVector2D>& TrapezoidPoints, const FVector2D ScaleUV, const FVector2D OffsetUV)
{
	TArray<FVector2D> PointsUV;
	const int NumPoints = TrapezoidPoints.Num();
	
	if (NumPoints < 4) return PointsUV;

	// [0] [2] one of the edges of the parallel sides of the trapezoid
	const FVector2D RotationCenter = TrapezoidPoints[0];
	
	const FVector2D Vec = TrapezoidPoints[2] - RotationCenter;

	// Angle between Vec and (1,0)
	float Angle = FMath::Atan2(Vec.Y, Vec.X);

	if (Angle < 0)
	{
		Angle += 2 * PI;
	}

	Angle *= -1;

	// First point (0,0) + offset
	PointsUV.Add(OffsetUV * ScaleUV);

	//Rotate other points, 
	for (int i = 1; i < 4; ++i)
	{
		const FVector2D Point = RotateVectorRadians(TrapezoidPoints[i] - RotationCenter, Angle);
		PointsUV.Add((Point + OffsetUV) * ScaleUV);
	}

	return PointsUV;
}

TArray<FVector2D> ASimplePathTracerActor::CalculateSimpleUV(const TArray<FVector2D>& Vertexes,
                                                            const float Thickness,
                                                            const FVector2D ScaleUV,
                                                            const FVector2D OffsetUV,
                                                            const bool bRemoveSeamsOnUV)
{
	const int NumVertexes = Vertexes.Num();
	
	if (NumVertexes < 4 || ScaleUV.X == 0 || ScaleUV.Y == 0) return {};
	
	TArray<FVector2D> UV;

	const int NumUV = NumVertexes / 4;

	const float DoubleThickness = Thickness * 2;
	
	float LastOffset = 0.0f;
	
	for (int i = 0; i < NumUV; ++i)
	{
		const int  k = i * 4;
		
		// (P0 - P2).Size()
		const float TopEdgeSize = (Vertexes[k] - Vertexes[k + 2]).Size();
		
		if(bRemoveSeamsOnUV)
		{
			const float U = TopEdgeSize / DoubleThickness * ScaleUV.X;
			const float V = ScaleUV.Y + OffsetUV.Y;

			// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
			UV.Add({LastOffset, OffsetUV.Y});
			UV.Add({LastOffset, V});
			UV.Add({LastOffset + U, OffsetUV.Y});
			UV.Add({LastOffset + U, V});
			
			LastOffset = FMath::Fmod(LastOffset + U, 1.f);

		}
		else
		{
			const float U = TopEdgeSize / DoubleThickness * ScaleUV.X + OffsetUV.X;
			const float V = ScaleUV.Y + OffsetUV.Y;
			
			// UV Quad: Left Top —> Left Bot —> Right Top —> Right Bot
			UV.Add({OffsetUV.X, OffsetUV.Y});
			UV.Add({OffsetUV.X, V});
			UV.Add({U, OffsetUV.Y});
			UV.Add({U, V});
		}
	}
	
	return UV;
}

TArray<FVector> ASimplePathTracerActor::BuildBezierCurve3D(const FVector& P0, const FVector& P1, const FVector& P2, const int NumPoints)
{
	TArray<FVector> CurvePoints;
	
	if (NumPoints <= 0) return CurvePoints;
	
	for (int i = 0; i < NumPoints; i++)
	{
		const float T = static_cast<float>(i) / static_cast<float>(NumPoints);
		FVector Point = CalculateBezierPoint3D(P0, P1, P1, P2, T);
		CurvePoints.Add(Point);
	}

	CurvePoints.Add(P2);
	
	return CurvePoints;
}

FVector ASimplePathTracerActor::CalculateBezierPoint3D(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T)
{
	float U = 1.0f - T;
	float TT = T * T;
	float UU = U * U;
	float UUU = UU * U;
	float TTT = TT * T;

	return UUU * P0 + 3 * UU * T * P1 + 3 * U * TT * P2 + TTT * P3;
}