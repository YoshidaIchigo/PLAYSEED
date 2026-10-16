#include "BattlefieldUI.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
FBattlefieldUI::FBattlefieldUI(UCanvas* InCanvas):Canvas(InCanvas)
{
    Scale=FMath::Min(Canvas->SizeX/320.f,Canvas->SizeY/240.f);
    Origin=FVector2D((Canvas->SizeX-320*Scale)*.5f,(Canvas->SizeY-240*Scale)*.5f);
}
FVector2D FBattlefieldUI::Screen(float X,float Y) const {return Origin+FVector2D(X,Y)*Scale;}
void FBattlefieldUI::Rect(float X,float Y,float W,float H,FLinearColor Color) const
{
    FCanvasTileItem Item(Screen(X,Y),FVector2D(W,H)*Scale,Color);Item.BlendMode=SE_BLEND_Translucent;Canvas->DrawItem(Item);
}
void FBattlefieldUI::Triangle(FVector2D A,FVector2D B,FVector2D C,FLinearColor Color) const
{
    FCanvasUVTri T;T.V0_Pos=Screen(A.X,A.Y);T.V1_Pos=Screen(B.X,B.Y);T.V2_Pos=Screen(C.X,C.Y);
    T.V0_UV=FVector2D::ZeroVector;T.V1_UV=FVector2D::ZeroVector;T.V2_UV=FVector2D::ZeroVector;
    T.V0_Color=T.V1_Color=T.V2_Color=Color;Canvas->K2_DrawTriangle(nullptr,{T});
}
void FBattlefieldUI::Ellipse(FVector2D P,FVector2D Radius,FLinearColor Color) const
{
    for(int32 I=0;I<32;++I){const float A=I*2*PI/32,B=(I+1)*2*PI/32;Triangle(P,P+FVector2D(FMath::Cos(A)*Radius.X,FMath::Sin(A)*Radius.Y),P+FVector2D(FMath::Cos(B)*Radius.X,FMath::Sin(B)*Radius.Y),Color);}
}
void FBattlefieldUI::Text(const FString& Value,float X,float Y,float Size,FLinearColor Color,bool bCenter,bool bOutline,bool bJapanese) const
{
    const FString Font=FPaths::EngineContentDir()/(bJapanese?TEXT("Slate/Fonts/DroidSansFallback.ttf"):TEXT("Slate/Fonts/Roboto-BlackItalic.ttf"));
    FCanvasTextItem Item(Screen(X,Y),FText::FromString(Value),FSlateFontInfo(Font,FMath::Max(6,FMath::RoundToInt(Size*Scale))),Color);
    Item.bCentreX=bCenter;Item.bOutlined=bOutline;Item.OutlineColor=FLinearColor::Black;
    if(bOutline)Item.EnableShadow(FLinearColor::Black,FVector2D(1,1)*Scale);
    Canvas->DrawItem(Item);
}
void FBattlefieldUI::Star(FVector2D P,float Radius,FLinearColor Color,bool bEyes) const
{
    // 外側5点と内側5点を交互に並べ、中心から10枚の三角形で星を塗ります。
    TArray<FVector2D> Points;
    for(int32 I=0;I<10;++I){const float A=-PI*.5f+I*PI/5,R=Radius*(I%2==0?1.f:.46f);Points.Add(P+FVector2D(FMath::Cos(A),FMath::Sin(A))*R);}
    for(int32 I=0;I<10;++I)Triangle(P,Points[I],Points[(I+1)%10],Color);
    if(bEyes){Rect(P.X-Radius*.21f,P.Y-Radius*.18f,Radius*.12f,Radius*.35f,FLinearColor::Black);Rect(P.X+Radius*.1f,P.Y-Radius*.18f,Radius*.12f,Radius*.35f,FLinearColor::Black);}
}
void FBattlefieldUI::Coin(FVector2D P,float Radius) const
{
    Ellipse(P,FVector2D(Radius*.78f,Radius),FLinearColor(.75f,.32f,.015f));
    Ellipse(P+FVector2D(-1,0),FVector2D(Radius*.6f,Radius*.84f),FLinearColor(1,.78f,.03f));
    Rect(P.X-2,P.Y-3,1,6,FLinearColor(.4f,.22f,.015f));Rect(P.X+1,P.Y-3,1,6,FLinearColor(.4f,.22f,.015f));
}
void FBattlefieldUI::Face(FVector2D P) const
{
    Ellipse(P+FVector2D(0,2),FVector2D(8,8),FLinearColor(.95f,.56f,.3f));
    Ellipse(P+FVector2D(0,-4),FVector2D(9,5),FLinearColor(.9f,.015f,.01f));Rect(P.X-9,P.Y-3,19,3,FLinearColor(1,.025f,.01f));
    Ellipse(P+FVector2D(0,-5),FVector2D(3,3),FLinearColor::White);Text(TEXT("M"),P.X,P.Y-8,4,FLinearColor::Red,true,false);
    Rect(P.X-4,P.Y,2,4,FLinearColor(.05f,.15f,.35f));Rect(P.X+2,P.Y,2,4,FLinearColor(.05f,.15f,.35f));
    Ellipse(P+FVector2D(0,5),FVector2D(6,2),FLinearColor(.08f,.025f,.015f));Ellipse(P+FVector2D(0,3),FVector2D(3,3),FLinearColor(1,.65f,.4f));
}
void FBattlefieldUI::Power(FVector2D P,int32 Health) const
{
    for(int32 I=0;I<8;++I){const float A=I*PI/4;Ellipse(P+FVector2D(FMath::Cos(A),FMath::Sin(A))*19,FVector2D(5,5),FLinearColor(.8f,.55f,.23f));}
    Ellipse(P,FVector2D(18,18),FLinearColor(.95f,.75f,.35f));Ellipse(P,FVector2D(15,15),FLinearColor(.02f,.025f,.04f));
    const FLinearColor Fill=Health>6?FLinearColor(.05f,.3f,1):Health>4?FLinearColor(.12f,.8f,.08f):Health>2?FLinearColor(1,.85f,.02f):FLinearColor(1,.06f,.01f);
    for(int32 I=0;I<FMath::Clamp(Health,0,8);++I)for(int32 K=0;K<5;++K){const float A=-PI*.5f+I*PI/4+.035f+K*(PI/4-.07f)/5,B=A+(PI/4-.07f)/5;Triangle(P,P+FVector2D(FMath::Cos(A),FMath::Sin(A))*14,P+FVector2D(FMath::Cos(B),FMath::Sin(B))*14,Fill);}
    const FLinearColor Colors[]={FLinearColor::Red,FLinearColor(1,.8f,0),FLinearColor(.1f,.9f,.2f),FLinearColor(.1f,.45f,1),FLinearColor(.8f,.15f,1)};
    const TCHAR* Letters[]={TEXT("P"),TEXT("O"),TEXT("W"),TEXT("E"),TEXT("R")};for(int32 I=0;I<5;++I)Text(Letters[I],P.X-19+I*8,P.Y-34,9,Colors[I]);
}
void FBattlefieldUI::Camera() const
{
    Rect(280,216,10,7,FLinearColor(.08f,.08f,.1f));Ellipse(FVector2D(282,215),FVector2D(3,3),FLinearColor(.05f,.05f,.05f));Ellipse(FVector2D(288,215),FVector2D(3,3),FLinearColor(.05f,.05f,.05f));
    Triangle(FVector2D(290,218),FVector2D(296,215),FVector2D(296,224),FLinearColor(.08f,.08f,.1f));Ellipse(FVector2D(304,221),FVector2D(7,5),FLinearColor(1,.65f,.02f));
    Rect(301,218,1.5f,3,FLinearColor::Black);Rect(305,218,1.5f,3,FLinearColor::Black);
}
