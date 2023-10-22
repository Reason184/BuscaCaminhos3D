// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

class FBenchmark
{
public:
    FBenchmark()
    {
        StartTimePoint = FPlatformTime::Seconds();
    }

    ~FBenchmark()
    {
        Stop();
    }

    void Stop()
    {
        double EndTimePoint = FPlatformTime::Seconds();
        double Duration = EndTimePoint - StartTimePoint;

        double Microseconds = Duration * 1e6;
        GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, FString::Printf(TEXT("Code Duration: %f microseconds"), Microseconds));
    }

private:
    double StartTimePoint;
};
