// Fill out your copyright notice in the Description page of Project Settings.


#include "BBPlayerState.h"
#include "Net/UnrealNetwork.h"

ABBPlayerState::ABBPlayerState()
	: PlayerNickName(TEXT("None")),
	CurrentNumberCount(0),
	MaxNumberCount(6),
	RemainingSeconds(0)
{
	bReplicates = true;
}

void ABBPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNickName);
	DOREPLIFETIME(ThisClass, CurrentNumberCount);
	DOREPLIFETIME(ThisClass, MaxNumberCount);
	DOREPLIFETIME(ThisClass, RemainingSeconds);
}

FString ABBPlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerNickName + TEXT("(") + FString::FromInt(CurrentNumberCount) + TEXT("/") + FString::FromInt(MaxNumberCount) + TEXT(")");
	return PlayerInfoString;
}