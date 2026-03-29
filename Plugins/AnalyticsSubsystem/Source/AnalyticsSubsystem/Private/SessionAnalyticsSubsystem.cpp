#include "SessionAnalyticsSubsystem.h"

void USessionAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    SessionID = GenerateSessionID();
    SessionStartTime = FDateTime::Now();

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Session Started: %s"),
        *SessionID);
}

void USessionAnalyticsSubsystem::Deinitialize()
{
    FTimespan Duration = FDateTime::Now() - SessionStartTime;

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Session Ended | Duration: %.2f seconds"),
        Duration.GetTotalSeconds());

    Super::Deinitialize();
}

void USessionAnalyticsSubsystem::LogEvent(const FString& EventName)
{
    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Event: %s"), *EventName);
}

FString USessionAnalyticsSubsystem::GenerateSessionID() const
{
    FDateTime Now = FDateTime::Now();

    return FString::Printf(
        TEXT("SESSION_%04d%02d%02d_%02d%02d%02d"),
        Now.GetYear(),
        Now.GetMonth(),
        Now.GetDay(),
        Now.GetHour(),
        Now.GetMinute(),
        Now.GetSecond());
}