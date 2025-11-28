// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BBGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class BASEBALL_API ABBGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCLogin(const FString& PlayerNameString = FString(TEXT("XXXXXXX")));
};
