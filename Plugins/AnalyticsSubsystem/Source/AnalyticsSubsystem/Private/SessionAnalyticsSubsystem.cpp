#include "SessionAnalyticsSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

void USessionAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    SessionID = GenerateSessionID();
    SessionStartTime = FDateTime::Now();
    LastEvent = TEXT("None");
    LastPositionText = TEXT("Unknown");
    RecentEvents.Empty();

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Session Started: %s"),
        *SessionID);

    StartPositionTracking();
}

void USessionAnalyticsSubsystem::Deinitialize()
{
    StopPositionTracking();

    const FTimespan Duration = FDateTime::Now() - SessionStartTime;

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Session Ended | Duration: %.2f seconds"),
        Duration.GetTotalSeconds());

    Super::Deinitialize();
}

void USessionAnalyticsSubsystem::LogEvent(const FString& EventName)
{
    LastEvent = EventName;
    RecentEvents.Add(EventName);

    if (RecentEvents.Num() > 8)
    {
        RecentEvents.RemoveAt(0);
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Event: %s"),
        *EventName);
}

void USessionAnalyticsSubsystem::LogPlayerPosition(AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        return;
    }

    const FVector Location = PlayerActor->GetActorLocation();

    LastPositionText = FString::Printf(
        TEXT("X=%.1f Y=%.1f Z=%.1f"),
        Location.X,
        Location.Y,
        Location.Z
    );

    RecentEvents.Add(FString::Printf(TEXT("Position | %s"), *LastPositionText));

    if (RecentEvents.Num() > 8)
    {
        RecentEvents.RemoveAt(0);
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Position | %s"),
        *LastPositionText);
}

FString USessionAnalyticsSubsystem::GetSessionID() const
{
    return SessionID;
}

float USessionAnalyticsSubsystem::GetSessionDurationSeconds() const
{
    return (FDateTime::Now() - SessionStartTime).GetTotalSeconds();
}

FString USessionAnalyticsSubsystem::GetLastEvent() const
{
    return LastEvent;
}

FString USessionAnalyticsSubsystem::GetLastPositionText() const
{
    return LastPositionText;
}

TArray<FString> USessionAnalyticsSubsystem::GetRecentEvents() const
{
    return RecentEvents;
}

FString USessionAnalyticsSubsystem::GenerateSessionID() const
{
    const FDateTime Now = FDateTime::Now();

    return FString::Printf(
        TEXT("SESSION_%04d%02d%02d_%02d%02d%02d"),
        Now.GetYear(),
        Now.GetMonth(),
        Now.GetDay(),
        Now.GetHour(),
        Now.GetMinute(),
        Now.GetSecond());
}

void USessionAnalyticsSubsystem::StartPositionTracking()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().SetTimer(
        PositionTimerHandle,
        this,
        &USessionAnalyticsSubsystem::TrackPlayerPosition,
        2.0f,
        true
    );
}

void USessionAnalyticsSubsystem::StopPositionTracking()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(PositionTimerHandle);
}

void USessionAnalyticsSubsystem::TrackPlayerPosition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    LogPlayerPosition(PlayerPawn);
}