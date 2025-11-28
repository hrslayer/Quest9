// Fill out your copyright notice in the Description page of Project Settings.


#include "BBPlayerController.h"
#include "BBHUDWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BBGameModeBase.h"
#include "BBPlayerState.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ABBPlayerController::ABBPlayerController()
{
	bReplicates = true;
}

void ABBPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InfoText);
}

void ABBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}
	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(WidgetClass) == true)
	{
		WidgetInstance = CreateWidget<UBBHUDWidget>(this, WidgetClass);
		if (IsValid(WidgetInstance) == true)
		{
			WidgetInstance->AddToViewport();
		}
	}
}

void ABBPlayerController::SetChattingMessage(const FString& InChattingMessage)
{
	ChattingMessage = InChattingMessage;

	if (IsLocalController() == true)
	{
		ABBPlayerState* BBPS = GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			FString ConnetionMessage = BBPS->GetPlayerInfoString() + TEXT(": ") + InChattingMessage;

			ServerRPC_PrintChatting(ConnetionMessage);
		}
	}
}

void ABBPlayerController::DisplayChattingMessage(const FString& InChattingMessage)
{
	UKismetSystemLibrary::PrintString(this, InChattingMessage, true, true, FLinearColor::White, 5.0f);
}

void ABBPlayerController::ClientRPC_PrintChatting_Implementation(const FString& InChattingMessage)
{
	DisplayChattingMessage(InChattingMessage);
}

void ABBPlayerController::ServerRPC_PrintChatting_Implementation(const FString& InChattingMessage)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ABBGameModeBase* BBGM = Cast<ABBGameModeBase>(GM);
		if (IsValid(BBGM) == true)
		{
			BBGM->PrintChattingMessage(this, InChattingMessage);
		}
	}
}