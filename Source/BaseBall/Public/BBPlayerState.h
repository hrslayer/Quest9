// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BBPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BASEBALL_API ABBPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ABBPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetPlayerInfoString();

public:
	UPROPERTY(Replicated)
	FString PlayerNickName;

	UPROPERTY(Replicated)
	int32 CurrentNumberCount;

	UPROPERTY(Replicated)
	int32 MaxNumberCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Turn")
	int32 RemainingSeconds;
};