#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionAnalyticsSubsystem.generated.h"

UCLASS()
class ANALYTICSSUBSYSTEM_API USessionAnalyticsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:

    FString SessionID;
    FDateTime SessionStartTime;

    FString GenerateSessionID() const;
};