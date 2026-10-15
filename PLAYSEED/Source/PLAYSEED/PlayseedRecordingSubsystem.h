#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayseedRecordingSubsystem.generated.h"

class FAVIWriter;
class UGameViewportClient;

// GameMode はレベル移動で破棄されますが、GameInstanceSubsystem はゲーム終了まで残ります。
// 録画だけをここで所有すると、ステージ選択からプレイまでを一つの動画で確認できます。
UCLASS()
class PLAYSEED_API UPlayseedRecordingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    bool StartRecording(const FString& SavedRelativeFilename);
    int32 FinishRecording();
    int32 GetCapturedFrameCount() const;
    bool IsRecording() const { return bRecording; }
    bool HasCaptureError() const { return bCaptureFailed; }
    virtual void Deinitialize() override;

private:
    void CaptureViewport();
    bool CreateWriter(const FIntPoint& Size);

    // Viewport の寿命は Engine が管理します。弱参照で監視し、ここでは延命しません。
    TWeakObjectPtr<UGameViewportClient> ObservedViewport;
    FDelegateHandle CaptureDrawHandle;
    // FAVIWriter は UObject ではないため、GC 任せにせず FinishRecording で解放します。
    FAVIWriter* CaptureWriter = nullptr;
    FString OutputFilename;
    FIntPoint RecordingSize = FIntPoint::ZeroValue;
    double StartedAt = 0;
    double NextCapture = 0;
    int32 FinishedFrameCount = 0;
    bool bRecording = false;
    bool bCaptureFailed = false;
};
