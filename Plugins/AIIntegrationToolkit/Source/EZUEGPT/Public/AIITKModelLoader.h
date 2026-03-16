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



 // =================== AIITKModelLoader.h ===================
 
#pragma once

#include "Misc/EngineVersion.h"
#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "OpenAIFunctionLibrary.h"
#include "AIITKModelLoader.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class EZUEGPT_API UAIITKModelLoader : public UProceduralMeshComponent
{
    GENERATED_BODY()

public:
    // Function to load a model from a given file path
    UFUNCTION(BlueprintCallable, Category = "ModelLoader")
    bool LoadModel(const FString& FilePath);

    // Function to parse OBJ file
    bool ParseOBJFile(const FString& FilePath, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FVector>& Tangents);

private:
    void CalculateTangents(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector2D>& UVs, TArray<FVector>& Tangents);
};
