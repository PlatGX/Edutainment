// Copyright 2023 Andrew Esenin. All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimplePathTracerActor.generated.h"

UCLASS()
class SIMPLEPATHTRACER_API ASimplePathTracerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimplePathTracerActor();

	/**
	 * Calculates the data for a regular path.
	 * Returns arrays with vertex, triangle and UV coordinates,
	 * connect these arrays to the "Create Mesh Section" function of the Procedural Mesh to create a path in the editor or in runtime.
	 *
	 * @param PathPoints Array of path points.
	 * @param Vertexes Array of vertex coordinates.
	 * @param Triangles Array of triangles.
	 * @param UV Array of UV coordinates.
	 * @param bLoopPath Loops the path.
	 * @param Thickness Half of the line thickness.
	 * @param Offset Offset vertexes from the center, offset works even if the path is not closed.
	 * @param bEnableUV Enables UV creation, enable if you want a unique texture on the meshes of your path, not just a color.
	 * @param ScaleUV Scale UV on two axes, use to, for example, make dashes more frequent or sparse.
	 * @param OffsetUV Shifts the UV, use to move the texture to the side or away from the center of the path meshes.
	 * @param bRemoveSeamsOnUV Removes texture seams between polygons.
	 * @param bRectangularUV Makes the UV of each polygon rectangular, if false UV will match the shape of the polygon.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	bool GetPathData(const TArray<FVector>& PathPoints,
	                 TArray<FVector>& Vertexes,
	                 TArray<int>& Triangles,
	                 TArray<FVector2D>& UV,
	                 const bool bLoopPath = false,
	                 const float Thickness = 10.0f,
	                 const float Offset = 0.0f,
	                 const bool bEnableUV = false,
	                 const FVector2D ScaleUV = FVector2D(1.0f, 1.0f),
	                 const FVector2D OffsetUV = FVector2D(0.0f, 0.0f),
	                 const bool bRemoveSeamsOnUV = true,
	                 const bool bRectangularUV = false);

	/**
	 * Calculates the data for a vertical path.
     * Returns arrays with vertex, triangle and UV coordinates,
     * connect these arrays to the "Create Mesh Section" function of the procedural mesh to create a path in the editor or in runtime.
	 *
	 * @param PathPoints Array of path points.
	 * @param Vertexes Array of vertex coordinates.
	 * @param Triangles Array of triangles.
	 * @param UV Array of UV coordinates.
	 * @param bLoopPath Loops the path.
	 * @param Height Height of the geometry.
	 * @param Offset Offset the entire path vertically.
	 * @param bEnableUV Enables UV creation, enable if you want a unique texture on the meshes of your path, not just a color.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	bool GetVerticalPathData(const TArray<FVector>& PathPoints,
						     TArray<FVector>& Vertexes,
						     TArray<int>& Triangles,
						     TArray<FVector2D>& UV,
						     const bool bLoopPath = false,
						     const float Height = 10.0f,
						     const float Offset = 0.0f,
						     const bool bEnableUV = false);

	/**
	 * Calculates data for a path consisting of individual polygons.
	 * Returns arrays with vertex, triangle and UV coordinates,
	 * connect these arrays to the "Create Mesh Section" function of the procedural mesh to create a path in the editor or in runtime.
	 *
	 * @param PathPoints Array of path points.
	 * @param Vertexes Array of vertex coordinates.
	 * @param Triangles Array of triangles.
	 * @param UV Array of UV coordinates.
	 * @param bLoopPath Loops the path.
	 * @param Interval Distance between polygons.
	 * @param Length Polygon length.
	 * @param Width Width of the polygon.
	 * @param DotDirection Allows you to rotate the texture direction by 90 degrees, takes values from 0 to 3.
	 * @param bEnableUV Enables UV creation, enable if you want a unique texture on the meshes of your path, not just a color.
	 * @param ScaleUV Scale UV on two axes, use to, for example, make dashes more frequent or sparse.
	 * @param OffsetUV Shifts the UV, use to move the texture to the side or away from the center of the path meshes.
	 * @param AddLastDot If true will create a point at the last point of the path.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	bool GetDottedPathData(const TArray<FVector>& PathPoints,
	                       TArray<FVector>& Vertexes,
	                       TArray<int>& Triangles,
	                       TArray<FVector2D>& UV,
	                       const bool bLoopPath = false,
	                       const float Interval = 10.0f,
	                       const float Length = 10.0f,
	                       const float Width = 2.0f,
	                       const int DotDirection = 0,
	                       const bool bEnableUV = false,
	                       const FVector2D ScaleUV = FVector2D(1.0f, 1.0f),
	                       const FVector2D OffsetUV = FVector2D(0.0f, 0.0f),
	                       const bool AddLastDot = false);

	/**
	 * Calculates data for a path with indents in the corners.
	 * Returns arrays with vertex, triangle and UV coordinates,
	 * connect these arrays to the "Create Mesh Section" function of the procedural mesh to create a path in the editor or in runtime.
	 *
	 * @param PathPoints Array of path points.
	 * @param Vertexes Array of vertex coordinates.
	 * @param Triangles Array of triangles.
	 * @param UV Array of UV coordinates.
	 * @param bLoopPath Loops the path.
	 * @param Thickness Half of the line thickness.
	 * @param Offset Offset vertexes from the center, offset works even if the path is not closed.
	 * @param bEnableUV Enables UV creation, enable if you want a unique texture on the meshes of your path, not just a color.
	 * @param DirectionUV Allows you to rotate the texture direction by 90 degrees, takes values from 0 to 3.
	 * @param ScaleUV Scale UV on two axes, use to, for example, make dashes more frequent or sparse.
	 * @param OffsetUV Shifts the UV, use to move the texture to the side or away from the center of the path meshes.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	bool GetPathWithCornersOffsetData(const TArray<FVector>& PathPoints,
	                                  TArray<FVector>& Vertexes,
	                                  TArray<int>& Triangles,
	                                  TArray<FVector2D>& UV,
	                                  const bool bLoopPath = false,
	                                  const float Thickness = 10.0f,
	                                  const float Offset = 20.0f,
	                                  const bool bEnableUV = false,
	                                  const int DirectionUV = 0,
	                                  const FVector2D ScaleUV = FVector2D(1.0f, 1.0f),
	                                  const FVector2D OffsetUV = FVector2D(0.0f, 0.0f));

	/**
	 * This function will be useful if you want to attach some object to the beginning of the path and adjust its offset relative to the beginning of the path.
	 * The function returns the parameters of the first segment of the path, the coordinates of the shifted point lying on the same line with the initial segment.
	 * Use the StartOffset parameter to move the first point of the path along the first segment of the path.
	 * Rotation allows you to rotate some object so that it is always rotated to the same place as the first part of the path.
	 * Direction will help you find an additional displacement along the segment, perpendicular to it, or calculate another Rotation.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static void GetStartPathSegment(const TArray<FVector>& PathPoints, const float StartOffset, bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction);

	/**
	 * This function will be useful if you want to attach some object to the ending of the path and adjust its offset relative to the ending of the path.
	 * The function returns the parameters of the last segment of the path, the coordinates of the shifted point lying on the same line with the initial segment.
	 * Use the EndOffset parameter to move the last point of the path along the first segment of the path.
	 * Rotation allows you to rotate some object so that it is always rotated to the same place as the first part of the path.
	 * Direction will help you find an additional displacement along the segment, perpendicular to it, or calculate another Rotation.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static void GetEndPathSegment(const TArray<FVector>& PathPoints, const float EndOffset, bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction);

	/**
	 * Rounding the corners of the path.
	 * The Radius parameter determines the strength of the rounding, and the Segments parameter determines how many polygons will be created at each corner.
	 * With Segments = 1 it works like a chamfer, just cutting corners, Segments = 8-16 is enough to make smooth and rounded corners.
	 * It is also handy to use with a small Radius and a value of 1 for Segments to trim very long sharp corners that can occur with sharp path turns.
	 * bLoopPath in this case looping means that two rounded corners will be added between the first and last point of the path.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> RoundingPathCorners(const TArray<FVector>& PathPoints, const float Radius, const int Segments, const bool bLoopPath, const bool bPath3D = true);

	/**
	 * Returns the length of the path.
	 * Can be used to measure distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable, BlueprintPure)
	static float GetPathLength(const TArray<FVector>& PathPoints);

	/**
	 * Returns a path bounded by the given distance, all points that are outside the distance will not be included in the final array.
	 * Can be used to limit the maximum length of the path you want to display.
	 * It can also be used to animate gradually filling the path.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> LimitPathByDistance(const TArray<FVector>& PathPoints, const float Distance);

	/**
	 * Same as LimitPathByDistance, but the value varies between 0 and 1,
	 * with 0 being the beginning of the path, 1 being the end of the path, and 0.5 being the middle of the path.
	 * Handy to use if you don't know the length of your path, or only need to draw a certain percentage of the path.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> LimitPathByPercent(const TArray<FVector>& PathPoints, const float Percent);

	/**
	 * Returns the coordinates of a point at a given distance along the path.
	 * It is convenient to use to move an object along the path or to bind to it.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static void GetPointAlongPath(const TArray<FVector>& PathPoints, const float Distance, const bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction);

	/**
	 * Same as GetPointAlongPath but changes the value from 0 to 1, 0 is the beginning of the path,
	 * 1 is the end of the path, 0.5 is the middle of the path.
	 * Handy to use if you don't know the length of your path, or need to get the coordinates
	 * of the middle or part of the path as a percentage.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static void GetPointAlongPathPercent(const TArray<FVector>& PathPoints, const float Percent, const bool bIn2D, FVector& Location, FRotator& Rotation, FVector& Direction);

	/**
	 * Merges path points that are closer together than MergedDistance.
	 * If your path has consecutive points with the same coordinates, this function will remove them.
	 * You can also use this function for optimization, if you have clusters of points along the path,
	 * you can combine them into one and then, for example, round the corners.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> MergePathPoints(const TArray<FVector>& PathPoints, const float MergedDistance);

	/**
	 * Shifts all waypoints outward or inward.
	 * Convenient to use for shifting a closed boundary outward or inward.
	 * Self-intersections are not taken into account, so large bias values may lead to unexpected results.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> OffsetPathPoints(const TArray<FVector>& PathPoints, const float Offset);

	/**
	 * A helper function that splits a Vector array into two: a Vector2D array and a Z coordinate array of each point.
	 * Given the features of cycles in blueprints, this function just works much faster if you want to convert a Vector to Vector2D.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void SplitVectorArray(const TArray<FVector>& Points3D, TArray<FVector2D>& Points2D,TArray<float>& PointsZ);

	/**
	 * The same as SplitVectorArray but in reverse, it combines it into one array.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void CombineVectorArray(const TArray<FVector2D>& Points2D, const TArray<float>& PointsZ, TArray<FVector>& Points3D);

	/**
	 * Returns an array of points along the path taken at a given distance from each other.
	 * Handy to use if you need to set something up along the path at a certain distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> GetPointsAlongPath(const TArray<FVector>& PathPoints, const float Interval = 10.0f, const bool bAddLastPoint = true);

	/**
	 * Same as GetPointsAlongPath,but in this case it also returns the direction of each point.
	 * It will be useful for 3D paths, when path segments can be directed up or down.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void GetPointsAlongPathDirected(const TArray<FVector>& PathPoints, TArray<FVector>& Points, TArray<FVector>& Directions,
									const float Interval = 10.0f,
									const bool bAddLastPoint = true);

	/**
	 * Function for correcting points that are vertically on top of each other (X and Y coordinates coincide),
	 * this function moves points away from each other along the path by a specified distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> FixVerticalPathPoints(const TArray<FVector>& PathPoints, const float Offset = 10.0f, const bool bFixOverlap = true, const float FixTolerance = 0.01f);

	/**
	 * Makes the Z coordinate of all points in the path equal to the Height value.
	 * Convenient if you need to align all points in a horizontal plane
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> AlignPathPointsInHeight(const TArray<FVector>& PathPoints, const float Height);

	/**
	 * If several points of your path lie on the same straight line, they will be deleted.
	 * Convenient to use to optimize your path before using more expensive functions such as corner rounding.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> RemovePathPointsLyingOnLine(const TArray<FVector>& PathPoints, const float Tolerance = 0.1f);

	/**
	 * Adds two points at a given distance to each corner of the path, the corner point itself is deleted.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	TArray<FVector> AddPathCornersOffset(const TArray<FVector>& PathPoints, const float Offset);

	/**
	 * Correctly merges the vertexes, triangles, and UV coordinates of two paths.
	 * Useful if you want to avoid creating multiple procedural meshes or creating multiple sections of procedural meshes.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void CombinePathsData(const TArray<FVector>& Vertexes1, const TArray<FVector>& Vertexes2,
						  const TArray<int>& Triangles1, const TArray<int>& Triangles2,
						  const TArray<FVector2D>& UV1, const TArray<FVector2D>& UV2,
						  TArray<FVector>& Vertexes, TArray<int>& Triangles, TArray<FVector2D>& UV);

	/**
	 * Trims the beginning of the path by the specified distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> TrimPathFromStart(const TArray<FVector>& PathPoints, const float TrimLength);

	/**
	 * Trims the points at the end of the path by the specified distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> TrimPathFromEnd(const TArray<FVector>& PathPoints, const float TrimLength);

	/**
	 * Deletes all closely spaced points that are at the end of the path.
	 * Can be used to accurately position the end of the path.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
    static TArray<FVector> RemoveSmallSegmentsCloseToEnd(const TArray<FVector>& PathPoints, const float Length, const bool bFromStart);

	/**
	 * Removes all closely spaced points located on both sides of the path.
	 * Can be used to accurately position track ends.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FVector> RemoveSmallSegmentsCloseToBothEnds(const TArray<FVector>& PathPoints,
	                                                          const bool bEnableStart, const float StartLength,
	                                                          const bool bEnableEnd, const float EndLength);

	/**
	 * Trims the path on both sides.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void TrimPathComplex(const TArray<FVector>& PathPoints,
	                     TArray<FVector>& TrimPoints,
	                     FVector& StartTrimPoint,
	                     FRotator& StartRotation,
	                     FVector& StartDirection,
	                     FVector& EndTrimPoint,
	                     FRotator& EndRotation,
	                     FVector& EndDirection,
	                     const bool bEnableStartTrim = true,
	                     const float StartTrim = 20.0f,
	                     const bool bEnableEndTrim = true,
	                     const float EndTrim = 20.0f);

	/**
	 * Returns the data to create a unit square of the specified size.
	 * Can be used to create more advanced start and end of path elements.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	void GetQuadData(TArray<FVector>& Vertexes,
	                 TArray<int>& Triangles,
	                 TArray<FVector2D>& UV,
	                 const FVector Center,
	                 const FVector Direction = FVector(1, 0, 0),
	                 const float Length = 10.0f,
	                 const float Width = 10.0f,
	                 const bool bEnableUV = true);

	/**
	 * Returns the unit vector of the midpoint of the angle formed by the three points, P2 the corner point.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable, BlueprintPure)
	static FVector2D FindCornerMidDirection(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3);

	/**
	 * Returns an array of vertex colors for the dynamic mesh.
	 * Used to create a color path and transparency.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static TArray<FLinearColor> GetVertexColorData(const TArray<FVector>& PathPoints,
	                                               const bool bEnableStartOpacity,
	                                               const float StartDistance,
	                                               const bool bEnableEndOpacity,
	                                               const float EndDistance,
	                                               const bool bEnableUV,
	                                               const UCurveLinearColor* ColorCurve,
	                                               const bool bUnitCurve = true,
	                                               const float Hardness = 1.f,
	                                               const FLinearColor DefaultColor = FLinearColor::Black);

	/**
	 * Divides the path into two parts, at a specified distance.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static void SplitPath(const TArray<FVector> PathPoints,
						  const float Distance,
						  TArray<FVector>& StartPart,
						  TArray<FVector>& EndPart);

	/**
	 * Checks whether a point is inside an area or not, in 2D.
	 */
	UFUNCTION(Category=SimplePathTracer, BlueprintCallable)
	static bool CheckIfPointLiesInsideArea(const TArray<FVector> AreaPoints, const FVector& Point);


private:
	static void CalculatePathNormals(const TArray<FVector2D>& PathPoints, TArray<FVector2D>& Normals, TArray<float>& SinAngles);

	static TArray<int> CalculateTriangles(const int NumVertexes, const bool bEnableUV);

	static TArray<FVector2D> CalculateUV(const TArray<FVector2D>& Vertexes,
	                                     const float Thickness,
	                                     const FVector2D ScaleUV,
	                                     const FVector2D OffsetUV,
	                                     const bool bRemoveSeamsOnUV);
	
	static TArray<FVector2D> CalculateSimpleUV(const TArray<FVector2D>& Vertexes,
	                                           const float Thickness,
	                                           const FVector2D ScaleUV,
	                                           const FVector2D OffsetUV,
	                                           const bool bRemoveSeamsOnUV);

	static TArray<FVector2D> CalculateHeightUV(const TArray<FVector2D>& Vertexes, const float Height);

	static void FindVertexes(const TArray<FVector2D>& Points2D,
							 const TArray<float>& PointsZ,
							 const TArray<float>& SinAngles,
							 const TArray<FVector2D>& Normals,
							 const bool bEnableUV,
							 const float Thickness,
							 const float Offset,
							 TArray<FVector2D>& Vertexes,
							 TArray<float>& VertexesZ);

	static void FindVerticalVertexes(const TArray<FVector>& Points,
									 const bool bLoopPath,
									 const bool bEnableUV,
									 const float Height,
									 const float Offset, TArray<FVector>& Vertexes);

	static void FindVertexesLoop(const TArray<FVector2D>& Points2D,
								 const TArray<float>& PointsZ,
								 const TArray<float>& SinAngles,
								 const TArray<FVector2D>& Normals,
								 const bool bEnableUV,
								 const float Thickness,
								 const float Offset,
								 TArray<FVector2D>& Vertexes,
								 TArray<float>& VertexesZ);

	static void CalculateQuad(const FVector& CenterLocation,
	                          const FVector& Direction,
	                          const float Length,
	                          const float Width,
	                          const bool bEnableUV,
	                          const FVector2D& ScaleUV,
	                          const int DirectionUV,
	                          const FVector2D& OffsetUV,
	                          TArray<FVector>& Vertexes,
	                          TArray<int>& Triangles,
	                          TArray<FVector2D>& UV);
	
	static FVector2D CalculateBezierPoint(const float T, const FVector2D& P0, const FVector2D& P1, const FVector2D& P2);

	static TArray<FVector2D> BuildBezierCurve(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, int NumPoints);

	static TArray<FVector2D> AlignTrapezoidUV(const TArray<FVector2D>& TrapezoidPoints, const FVector2D ScaleUV, const FVector2D OffsetUV);
	
	static bool FindLineIntersection(const FVector2D& Start1, const FVector2D& End1, const FVector2D& Start2, const FVector2D& End2, FVector2D& IntersectionPoint);

	static FVector2D RotateVectorRadians(const FVector2D Vector, const float Angle);
	
	static float FindAngle360(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3);

	static TArray<FVector> BuildBezierCurve3D(const FVector& P0, const FVector& P1, const FVector& P2, int NumPoints);

	static FVector CalculateBezierPoint3D(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T);
	
};