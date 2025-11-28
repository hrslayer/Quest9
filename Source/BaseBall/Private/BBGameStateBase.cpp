// Fill out your copyright notice in the Description page of Project Settings.


#include "BBGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "BBPlayerController.h"

void ABBGameStateBase::MulticastRPCLogin_Implementation(const FString& PlayerNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ABBPlayerController* BBPC = Cast<ABBPlayerController>(PC);
			if (IsValid(BBPC) == true)
			{
				FString MessageAlert = PlayerNameString + TEXT("In Joing Game");
				BBPC->DisplayChattingMessage(MessageAlert);
			}
		}
	}
}