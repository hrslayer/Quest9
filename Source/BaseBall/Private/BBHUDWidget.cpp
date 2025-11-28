// Fill out your copyright notice in the Description page of Project Settings.


#include "BBHUDWidget.h"
#include "Components/EditableTextBox.h"
#include "BBPlayerController.h"
#include "Components/TextBlock.h"
#include "BBPlayerState.h"
#include "BBGameStateBase.h"
#include "Kismet/GameplayStatics.h"

void UBBHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatTextBox->OnTextCommitted.IsAlreadyBound(this, &ThisClass::ChattingTextInput) == false)
	{
		ChatTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::ChattingTextInput);
	}

	if (TimerText) TimerText->SetText(FText::FromString(TEXT("Waiting...")));
	if (StatusText) StatusText->SetText(FText::GetEmpty());
}

void UBBHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateGameInfo();
}

void UBBHUDWidget::NativeDestruct()
{
	if (ChatTextBox->OnTextCommitted.IsAlreadyBound(this, &ThisClass::ChattingTextInput) == true)
	{
		ChatTextBox->OnTextCommitted.RemoveDynamic(this, &ThisClass::ChattingTextInput);
	}
}

void UBBHUDWidget::ChattingTextInput(const FText& Text, ETextCommit::Type Commithod)
{
	if (Commithod == ETextCommit::OnEnter)
	{
		APlayerController* OPC = GetOwningPlayer();
		if (IsValid(OPC) == true)
		{
			ABBPlayerController* BaseballPC = Cast<ABBPlayerController>(OPC);
			if (IsValid(BaseballPC) == true)
			{
				BaseballPC->SetChattingMessage(Text.ToString());

				ChatTextBox->SetText(FText());
			}
		}
	}
}

void UBBHUDWidget::UpdateGameInfo()
{
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(PC))
	{
		ABBPlayerState* PS = PC->GetPlayerState<ABBPlayerState>();
		if (IsValid(PS))
		{
			if (TimerText)
			{
				FString TimeStr = FString::Printf(TEXT("Time Left: %d"), PS->RemainingSeconds);

				if (PS->RemainingSeconds <= 5 && PS->RemainingSeconds > 0)
				{
					TimerText->SetColorAndOpacity(FLinearColor::Red);
				}
				else
				{
					TimerText->SetColorAndOpacity(FLinearColor::White);
				}

				TimerText->SetText(FText::FromString(TimeStr));
			}

			if (PlayerInfoText)
			{
				FString InfoStr = FString::Printf(TEXT("My Try: %d / %d"), PS->CurrentNumberCount, PS->MaxNumberCount);
				PlayerInfoText->SetText(FText::FromString(InfoStr));
			}
		}
	}

	if (IsValid(PC))
	{
		ABBPlayerController* BBPC = Cast<ABBPlayerController>(PC);
		if (IsValid(BBPC))
		{
			if (StatusText)
			{
				StatusText->SetText(BBPC->InfoText);
			}
		}
	}
}