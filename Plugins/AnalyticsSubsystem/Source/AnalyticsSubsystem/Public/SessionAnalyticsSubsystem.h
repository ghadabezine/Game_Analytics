#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "SessionAnalyticsSubsystem.generated.h"

class AActor;
class UWorld;

UCLASS()
class ANALYTICSSUBSYSTEM_API USessionAnalyticsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void StartBackendSession(const FString& PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void EndBackendSession();

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void LogEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void LogPlayerPosition(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    FString GetSessionID() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    float GetSessionDurationSeconds() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    FString GetLastEvent() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    FString GetLastPositionText() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    TArray<FString> GetRecentEvents() const;

private:
    FString SessionID;
    FDateTime SessionStartTime;

    FString LastEvent;
    FString LastPositionText;
    TArray<FString> RecentEvents;

    FTimerHandle PositionTimerHandle;

    FString BackendBaseUrl = TEXT("http://127.0.0.1:5000/api/analytics");

    FString GenerateSessionID() const;
    void StartPositionTracking();
    void StopPositionTracking();
    void TrackPlayerPosition();

    void SendPostRequest(const FString& Endpoint, const FString& JsonBody);
    void OnStartSessionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};