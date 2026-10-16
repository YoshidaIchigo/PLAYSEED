#pragma once
#include "CoreMinimal.h"
class UCanvas;

// 320x240を基準に描き、4:3のゲーム画面とHUDの位置をそろえる小さな描画部品です。
// ゲームルールや保存値は持たず、HUDから渡された値だけを表示します。
class FBattlefieldUI
{
public:
    explicit FBattlefieldUI(UCanvas* InCanvas);
    void Rect(float X,float Y,float W,float H,FLinearColor Color) const;
    void Triangle(FVector2D A,FVector2D B,FVector2D C,FLinearColor Color) const;
    void Ellipse(FVector2D P,FVector2D Radius,FLinearColor Color) const;
    void Text(const FString& Value,float X,float Y,float Size,FLinearColor Color=FLinearColor::White,bool bCenter=false,bool bOutline=true,bool bJapanese=false) const;
    void Star(FVector2D P,float Radius,FLinearColor Color,bool bEyes=true) const;
    void Coin(FVector2D P,float Radius=8) const;
    void Face(FVector2D P) const;
    void Power(FVector2D P,int32 Health) const;
    void Camera() const;
    FVector2D Screen(float X,float Y) const;
    float GetScale() const { return Scale; }
private:
    UCanvas* Canvas;
    FVector2D Origin;
    float Scale;
};
