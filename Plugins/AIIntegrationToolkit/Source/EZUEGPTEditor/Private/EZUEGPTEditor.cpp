// Copyright 2023 Kaleb Knoettgen. All rights reserved.

#include "EZUEGPTEditor.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FEZUEGPTEditorModule"

DEFINE_LOG_CATEGORY(AIITKEditorLog);


void FEZUEGPTEditorModule::StartupModule()
{
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    MenuExtender = MakeShareable(new FExtender);
    MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FEZUEGPTEditorModule::AddMenuEntry));

    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FEZUEGPTEditorModule::ShutdownModule()
{
    if (MenuExtender.IsValid())
    {
        if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
        {
            FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
            LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
        }

        MenuExtender.Reset();
    }
}

void FEZUEGPTEditorModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString("AI Integration Toolkit API Keys"),
        FText::FromString("Open window with useful API key links"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FEZUEGPTEditorModule::OpenAPIKeysWindow))
    );
}

void FEZUEGPTEditorModule::OpenAPIKeysWindow()
{
    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString("API Keys"))
        .ClientSize(FVector2D(400.f, 120.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Padding(5)
            .AutoHeight()
            [
                SNew(SButton)
                .Text(FText::FromString("OpenAI API Keys"))
                .OnClicked_Lambda([]()->FReply
                {
                    FPlatformProcess::LaunchURL(TEXT("https://platform.openai.com/api-keys"), nullptr, nullptr);
                    return FReply::Handled();
                })
            ]
            + SVerticalBox::Slot()
            .Padding(5)
            .AutoHeight()
            [
                SNew(SButton)
                .Text(FText::FromString("ElevenLabs API Keys"))
                .OnClicked_Lambda([]()->FReply
                {
                    FPlatformProcess::LaunchURL(TEXT("https://elevenlabs.io/app/settings/api-keys"), nullptr, nullptr);
                    return FReply::Handled();
                })
            ]
        ];

    FSlateApplication::Get().AddWindow(Window);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEZUEGPTEditorModule, EZUEGPTEditor)
