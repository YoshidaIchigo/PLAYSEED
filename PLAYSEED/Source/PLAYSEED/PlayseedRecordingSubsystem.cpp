#include "PlayseedRecordingSubsystem.h"
#include "AVIWriter.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "UnrealClient.h"

bool UPlayseedRecordingSubsystem::StartRecording(const FString& SavedRelativeFilename)
{
#if !UE_BUILD_SHIPPING
    // 移動先の GameMode が再び開始を頼んでも、進行中のファイルを上書きしません。
    if (bRecording) return !bCaptureFailed;
    if (!GEngine || !GEngine->GameViewport) return false;
    OutputFilename = FPaths::ProjectSavedDir() / SavedRelativeFilename;
    FinishedFrameCount = 0;
    bCaptureFailed = false;
    StartedAt = FPlatformTime::Seconds();
    NextCapture = .5;
    ObservedViewport = GEngine->GameViewport;
    CaptureDrawHandle = ObservedViewport->OnEndDraw().AddUObject(this, &UPlayseedRecordingSubsystem::CaptureViewport);
    bRecording = true;
    return true;
#else
    return false;
#endif
}

bool UPlayseedRecordingSubsystem::CreateWriter(const FIntPoint& Size)
{
    FAVIWriterOptions Options;
    Options.OutputFilename = OutputFilename;
    Options.Width = Size.X;
    Options.Height = Size.Y;
    Options.CaptureFramerateNumerator = 15;
    Options.CompressionQuality = .75f;
    Options.bSynchronizeFrames = true;
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputFilename), true);
    CaptureWriter = FAVIWriter::CreateInstance(Options);
    if (CaptureWriter) CaptureWriter->Initialize();
    RecordingSize = Size;
    if (CaptureWriter && CaptureWriter->IsCapturing()) return true;
    bCaptureFailed = true;
    UE_LOG(LogTemp, Error, TEXT("PLAYSEED CAPTURE: AVI capture could not start."));
    return false;
}

void UPlayseedRecordingSubsystem::CaptureViewport()
{
    if (!bRecording || bCaptureFailed || !ObservedViewport.IsValid() || !ObservedViewport->Viewport) return;
    // World の時刻はレベルごとにゼロへ戻るため、録画にはプロセスの単調増加時計を使います。
    const double CaptureTime = FPlatformTime::Seconds() - StartedAt;
    if (CaptureTime < NextCapture) return;
    FViewport* Viewport = ObservedViewport->Viewport;
    const FIntPoint Size = Viewport->GetSizeXY();
    if (Size.X <= 0 || Size.Y <= 0) return;
    if (!CaptureWriter && !CreateWriter(Size)) return;
    if (Size != RecordingSize)
    {
        bCaptureFailed = true;
        UE_LOG(LogTemp, Error, TEXT("PLAYSEED CAPTURE: viewport size changed during recording."));
        return;
    }
    TArray<FColor> Pixels;
    // 描画テクスチャは Render Thread 側で取得します。Flush が終わるまで Pixels の参照は有効です。
    ENQUEUE_RENDER_COMMAND(PlayseedReadback)([Viewport, Size, &Pixels](FRHICommandListImmediate& RHICmdList)
    {
        const FTextureRHIRef& Texture = Viewport->GetShaderResourceTexture();
        if (Texture.IsValid())
            RHICmdList.ReadSurfaceData(Texture, FIntRect(0, 0, Size.X, Size.Y), Pixels, FReadSurfaceDataFlags(RCM_UNorm));
    });
    FlushRenderingCommands();
    if (Pixels.Num() == Size.X * Size.Y) CaptureWriter->Update(CaptureTime, MoveTemp(Pixels));
    NextCapture = CaptureTime + 1.0 / 15.0;
}

int32 UPlayseedRecordingSubsystem::GetCapturedFrameCount() const
{
    return CaptureWriter ? CaptureWriter->GetFrameNumber() : FinishedFrameCount;
}

int32 UPlayseedRecordingSubsystem::FinishRecording()
{
    if (ObservedViewport.IsValid()) ObservedViewport->OnEndDraw().Remove(CaptureDrawHandle);
    CaptureDrawHandle.Reset();
    ObservedViewport.Reset();
    if (CaptureWriter)
    {
        FinishedFrameCount = CaptureWriter->GetFrameNumber();
        CaptureWriter->Finalize();
        delete CaptureWriter;
        CaptureWriter = nullptr;
    }
    bRecording = false;
    return FinishedFrameCount;
}

void UPlayseedRecordingSubsystem::Deinitialize()
{
    // 正常な検証終了以外にも、ゲーム終了時にファイルを閉じてヘッダーを確定させます。
    FinishRecording();
    Super::Deinitialize();
}
