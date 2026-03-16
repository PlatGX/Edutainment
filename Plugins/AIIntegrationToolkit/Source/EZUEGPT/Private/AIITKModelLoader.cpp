/********************************************************************
 * Copyright (C) 2024 Kaleb Knoettgen
 * All Rights Reserved.
 *
 * Unauthorized reproduction, modification, or distribution of this
 * software is prohibited. Provided "AS IS" without warranty; Kaleb
 * Knoettgen is not liable for any damages arising from its use.
 * All rights not expressly granted are reserved.
 *
 * For more information, visit: www.kalebknoettgen.com
 * Contact: kalebknoettgen@gmail.com
 ********************************************************************/



 // =================== AIITKModelLoader.cpp ===================

#include "AIITKModelLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"

bool UAIITKModelLoader::LoadModel(const FString& FilePath)
{
    UE_LOG(AIITKLog, Log, TEXT("LoadModel called with FilePath: %s"), *FilePath);

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FVector> Tangents;

    if (ParseOBJFile(FilePath, Vertices, Triangles, Normals, UVs, Tangents))
    {
        UE_LOG(AIITKLog, Log, TEXT("Parsed OBJ file successfully. Creating mesh section..."));

        TArray<FProcMeshTangent> ProcTangents;
        for (const FVector& Tangent : Tangents)
        {
            ProcTangents.Emplace(Tangent, false); // false indicates tangent is not bi-normal flipped
        }

        CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), ProcTangents, true);
        UE_LOG(AIITKLog, Log, TEXT("Mesh section created successfully."));
        return true;
    }
    else
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to load model from %s"), *FilePath);
        return false;
    }
}

bool UAIITKModelLoader::ParseOBJFile(const FString& FilePath, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FVector>& Tangents)
{
    UE_LOG(AIITKLog, Log, TEXT("ParseOBJFile called with FilePath: %s"), *FilePath);

    TArray<FString> FileLines;
    if (!FFileHelper::LoadFileToStringArray(FileLines, *FilePath))
    {
        UE_LOG(AIITKLog, Error, TEXT("Failed to load file: %s"), *FilePath);
        return false;
    }

    TArray<FVector> TempVertices;
    TArray<FVector> TempNormals;
    TArray<FVector2D> TempUVs;
    TArray<int32> TempVertexIndices;
    TArray<int32> TempNormalIndices;
    TArray<int32> TempUVIndices;

    for (const FString& Line : FileLines)
    {
        if (Line.StartsWith(TEXT("v ")))
        {
            FVector Vertex;
            TArray<FString> VertexComponents;
            Line.Mid(2).ParseIntoArray(VertexComponents, TEXT(" "), true);
            if (VertexComponents.Num() == 3)
            {
                Vertex.X = FCString::Atof(*VertexComponents[0]);
                Vertex.Y = FCString::Atof(*VertexComponents[1]);
                Vertex.Z = FCString::Atof(*VertexComponents[2]);
                TempVertices.Add(Vertex);
                //UE_LOG(AIITKLog, Log, TEXT("Parsed vertex: %s"), *Vertex.ToString());
            }
        }
        else if (Line.StartsWith(TEXT("vn ")))
        {
            FVector Normal;
            TArray<FString> NormalComponents;
            Line.Mid(3).ParseIntoArray(NormalComponents, TEXT(" "), true);
            if (NormalComponents.Num() == 3)
            {
                Normal.X = FCString::Atof(*NormalComponents[0]);
                Normal.Y = FCString::Atof(*NormalComponents[1]);
                Normal.Z = FCString::Atof(*NormalComponents[2]);
                TempNormals.Add(Normal);
                //UE_LOG(AIITKLog, Log, TEXT("Parsed normal: %s"), *Normal.ToString());
            }
        }
        else if (Line.StartsWith(TEXT("vt ")))
        {
            FVector2D UV;
            TArray<FString> UVComponents;
            Line.Mid(3).ParseIntoArray(UVComponents, TEXT(" "), true);
            if (UVComponents.Num() == 2)
            {
                UV.X = FCString::Atof(*UVComponents[0]);
                UV.Y = FCString::Atof(*UVComponents[1]);
                // Flip the Y-coordinate for UVs
                UV.Y = 1.0f - UV.Y;
                TempUVs.Add(UV);
                //UE_LOG(AIITKLog, Log, TEXT("Parsed UV: %s"), *UV.ToString());
            }
        }
        else if (Line.StartsWith(TEXT("f ")))
        {
            TArray<FString> Tokens;
            Line.ParseIntoArrayWS(Tokens);

            for (int32 i = 3; i > 0; --i) // Reversed order for front and back correction
            {
                TArray<FString> Indices;
                Tokens[i].ParseIntoArray(Indices, TEXT("/"), true);

                if (Indices.Num() > 0)
                {
                    int32 VertexIndex = FCString::Atoi(*Indices[0]) - 1;
                    if (TempVertices.IsValidIndex(VertexIndex))
                    {
                        TempVertexIndices.Add(VertexIndex);
                        //UE_LOG(AIITKLog, Log, TEXT("Parsed vertex index: %d"), VertexIndex);
                    }
                    else
                    {
                        UE_LOG(AIITKLog, Error, TEXT("Invalid vertex index: %d"), VertexIndex);
                    }
                }

                if (Indices.Num() > 1)
                {
                    int32 UVIndex = FCString::Atoi(*Indices[1]) - 1;
                    if (TempUVs.IsValidIndex(UVIndex))
                    {
                        TempUVIndices.Add(UVIndex);
                        //UE_LOG(AIITKLog, Log, TEXT("Parsed UV index: %d"), UVIndex);
                    }
                    else
                    {
                        UE_LOG(AIITKLog, Error, TEXT("Invalid UV index: %d"), UVIndex);
                    }
                }

                if (Indices.Num() > 2)
                {
                    int32 NormalIndex = FCString::Atoi(*Indices[2]) - 1;
                    if (TempNormals.IsValidIndex(NormalIndex))
                    {
                        TempNormalIndices.Add(NormalIndex);
                        //UE_LOG(AIITKLog, Log, TEXT("Parsed normal index: %d"), NormalIndex);
                    }
                    else
                    {
                        UE_LOG(AIITKLog, Error, TEXT("Invalid normal index: %d"), NormalIndex);
                    }
                }
            }
        }
    }

    // Populate final arrays
    for (int32 Index : TempVertexIndices)
    {
        if (TempVertices.IsValidIndex(Index))
        {
            Vertices.Add(TempVertices[Index]);
        }
    }

    for (int32 Index : TempUVIndices)
    {
        if (TempUVs.IsValidIndex(Index))
        {
            UVs.Add(TempUVs[Index]);
        }
    }

    for (int32 Index : TempNormalIndices)
    {
        if (TempNormals.IsValidIndex(Index))
        {
            Normals.Add(TempNormals[Index]);
        }
    }

    for (int32 i = 0; i < TempVertexIndices.Num(); ++i)
    {
        Triangles.Add(i);
    }

    // Calculate tangents
    CalculateTangents(Vertices, Triangles, UVs, Tangents);

    UE_LOG(AIITKLog, Log, TEXT("OBJ file parsed successfully with %d vertices, %d triangles"), Vertices.Num(), Triangles.Num());

    return true;
}

void UAIITKModelLoader::CalculateTangents(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector2D>& UVs, TArray<FVector>& Tangents)
{
    Tangents.SetNumZeroed(Vertices.Num());
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        int32 Index0 = Triangles[i];
        int32 Index1 = Triangles[i + 1];
        int32 Index2 = Triangles[i + 2];

        const FVector& P0 = Vertices[Index0];
        const FVector& P1 = Vertices[Index1];
        const FVector& P2 = Vertices[Index2];

        const FVector2D& UV0 = UVs[Index0];
        const FVector2D& UV1 = UVs[Index1];
        const FVector2D& UV2 = UVs[Index2];

        FVector Edge1 = P1 - P0;
        FVector Edge2 = P2 - P0;
        FVector2D DeltaUV1 = UV1 - UV0;
        FVector2D DeltaUV2 = UV2 - UV0;

        float f = 1.0f / (DeltaUV1.X * DeltaUV2.Y - DeltaUV2.X * DeltaUV1.Y);

        FVector Tangent;
        Tangent.X = f * (DeltaUV2.Y * Edge1.X - DeltaUV1.Y * Edge2.X);
        Tangent.Y = f * (DeltaUV2.Y * Edge1.Y - DeltaUV1.Y * Edge2.Y);
        Tangent.Z = f * (DeltaUV2.Y * Edge1.Z - DeltaUV1.Y * Edge2.Z);

        Tangents[Index0] += Tangent;
        Tangents[Index1] += Tangent;
        Tangents[Index2] += Tangent;
    }

    for (FVector& Tangent : Tangents)
    {
        Tangent.Normalize();
    }
}
