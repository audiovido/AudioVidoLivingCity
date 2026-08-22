#include "AudioVidoAgentStyle.h"

#include "Brushes/SlateColorBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyle.h"

TSharedPtr<FSlateStyleSet> FAudioVidoAgentStyle::StyleInstance;

FName FAudioVidoAgentStyle::GetStyleSetName()
{
    static const FName Name(TEXT("AudioVidoAgentStyle"));
    return Name;
}

void FAudioVidoAgentStyle::Initialize()
{
    if (StyleInstance.IsValid())
    {
        return;
    }

    StyleInstance = MakeShared<FSlateStyleSet>(GetStyleSetName());
    StyleInstance->Set("AudioVido.Background", new FSlateColorBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("0D1117")))));
    StyleInstance->Set("AudioVido.Surface", new FSlateColorBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("131A22")))));
    StyleInstance->Set("AudioVido.Elevated", new FSlateColorBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("202B36")))));
    StyleInstance->Set("AudioVido.Hover", new FSlateColorBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("26313D")))));
    StyleInstance->Set("AudioVido.Divider", new FSlateColorBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("32404F")))));
    StyleInstance->Set("AudioVido.Cyan", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("52D2FF"))));
    StyleInstance->Set("AudioVido.Violet", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("A741E9"))));
    StyleInstance->Set("AudioVido.Success", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("60D89D"))));
    StyleInstance->Set("AudioVido.Warning", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFC331"))));
    StyleInstance->Set("AudioVido.Error", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF6B6B"))));
    StyleInstance->Set("AudioVido.Text", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F2F6FA"))));
    StyleInstance->Set("AudioVido.TextMuted", FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("8A93A4"))));
    FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FAudioVidoAgentStyle::Shutdown()
{
    if (StyleInstance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
        StyleInstance.Reset();
    }
}

const ISlateStyle& FAudioVidoAgentStyle::Get()
{
    return *StyleInstance;
}
