#pragma once

#include "CoreMinimal.h"

class FSlateStyleSet;

class FAudioVidoAgentStyle
{
public:
    static void Initialize();
    static void Shutdown();
    static const ISlateStyle& Get();
    static FName GetStyleSetName();

private:
    static TSharedPtr<FSlateStyleSet> StyleInstance;
};
