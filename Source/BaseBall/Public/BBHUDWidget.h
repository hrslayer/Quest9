// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BBHUDWidget.generated.h"

class UEditableTextBox;
class UTextBlock;

UCLASS()
class BASEBALL_API UBBHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void ChattingTextInput(const FText& Text, ETextCommit::Type CommitMethod);

	void UpdateGameInfo();

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ChatTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerInfoText;
};