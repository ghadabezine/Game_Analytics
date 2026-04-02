#include "SessionAnalyticsSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Json.h"
#include "JsonUtilities.h"

void USessionAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    SessionID = TEXT("");
    SessionStartTime = FDateTime::Now();
    LastEvent = TEXT("None");
    LastPositionText = TEXT("Unknown");
    RecentEvents.Empty();

    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Subsystem initialized"));

    StartPositionTracking();
}

void USessionAnalyticsSubsystem::Deinitialize()
{
    StopPositionTracking();
    EndBackendSession();

    const FTimespan Duration = FDateTime::Now() - SessionStartTime;

    UE_LOG(LogTemp, Warning,
        TEXT("[Analytics] Session Ended | Duration: %.2f seconds"),
        Duration.GetTotalSeconds());

    Super::Deinitialize();
}

void USessionAnalyticsSubsystem::StartBackendSession(const FString& PlayerName)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("playerName"), PlayerName);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendBaseUrl + TEXT("/session/start"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &USessionAnalyticsSubsystem::OnStartSessionResponse);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Sending session start request"));
}

void USessionAnalyticsSubsystem::OnStartSessionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Analytics] Failed to start backend session"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Start session response: %s"), *Response->GetContentAsString());

    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid())
    {
        if (JsonResponse->HasField(TEXT("_id")))
        {
            SessionID = JsonResponse->GetStringField(TEXT("_id"));
            SessionStartTime = FDateTime::Now();

            UE_LOG(LogTemp, Warning, TEXT("[Analytics] Backend session started. ID: %s"), *SessionID);
        }
    }
}

void USessionAnalyticsSubsystem::EndBackendSession()
{
    if (SessionID.IsEmpty())
    {
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("sessionId"), SessionID);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendPostRequest(TEXT("/session/end"), RequestBody);
}

void USessionAnalyticsSubsystem::LogEvent(const FString& EventName)
{
    LastEvent = EventName;
    RecentEvents.Add(EventName);

    if (RecentEvents.Num() > 8)
    {
        RecentEvents.RemoveAt(0);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Event: %s"), *EventName);

    if (SessionID.IsEmpty())
    {
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("sessionId"), SessionID);
    JsonObject->SetStringField(TEXT("eventType"), EventName);
    JsonObject->SetStringField(TEXT("value"), TEXT("Triggered"));

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendPostRequest(TEXT("/event"), RequestBody);
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

    UE_LOG(LogTemp, Warning, TEXT("[Analytics] Position: %s"), *LastPositionText);

    if (SessionID.IsEmpty())
    {
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField(TEXT("sessionId"), SessionID);
    JsonObject->SetNumberField(TEXT("x"), Location.X);
    JsonObject->SetNumberField(TEXT("y"), Location.Y);
    JsonObject->SetNumberField(TEXT("z"), Location.Z);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    SendPostRequest(TEXT("/position"), RequestBody);
}

FString USessionAnalyticsSubsystem::GetSessionID() const
{
    return SessionID;
}

float USessionAnalyticsSubsystem::GetSessionDurationSeconds() const
{
    if (SessionStartTime == FDateTime())
    {
        return 0.f;
    }

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
        Now.GetSecond()
    );
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

void USessionAnalyticsSubsystem::SendPostRequest(const FString& Endpoint, const FString& JsonBody)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendBaseUrl + Endpoint);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(JsonBody);

    Request->OnProcessRequestComplete().BindLambda(
        [Endpoint](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (!bWasSuccessful || !Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("[Analytics] Request failed for %s"), *Endpoint);
                return;
            }

            UE_LOG(LogTemp, Warning, TEXT("[Analytics] %s -> %s"), *Endpoint, *Response->GetContentAsString());
        }
    );

    Request->ProcessRequest();
}