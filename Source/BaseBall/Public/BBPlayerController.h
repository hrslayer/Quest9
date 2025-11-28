// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BBPlayerController.generated.h"

class UBBHUDWidget;

UCLASS()
class BASEBALL_API ABBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABBPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	void SetChattingMessage(const FString& InChattingMessage);

	void DisplayChattingMessage(const FString& InChattingMessage);

	UFUNCTION(Client, Reliable)
	void ClientRPC_PrintChatting(const FString& InChattingMessage);

	UFUNCTION(Server, Reliable)
	void ServerRPC_PrintChatting(const FString& InChattingMessage);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBBHUDWidget> WidgetClass;

	UPROPERTY()
	TObjectPtr<UBBHUDWidget> WidgetInstance;

	FString ChattingMessage;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText InfoText;
};