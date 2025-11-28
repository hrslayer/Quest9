// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BBGameModeBase.generated.h"

class ABBPlayerController;

UCLASS()
class BASEBALL_API ABBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABBGameModeBase();

	virtual void BeginPlay() override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	virtual void OnPostLogin(AController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void PrintChattingMessage(ABBPlayerController* ChattingPlayerController, const FString& InChattingMessage);

	void StartGame();

protected:
	FString MakeSecretNumber();
	bool IsValidGuessNumber(const FString& InGuessNumberString);
	FString MakeGuessResult(const FString& SecretNumberString, const FString& GuessNumberString, int32& OutStrikeCount);

	void UpGuessCount(ABBPlayerController* InChattingPlayerController);
	void ResetGame();
	void JudgeGame(ABBPlayerController* InChattingPlayerController, int StrikeCount);

	UFUNCTION()
	void TickTurnTimer();

	void UpdateAllPlayersTimer();

protected:
	FString NumberRightAnswer;

	UPROPERTY()
	TArray<TObjectPtr<ABBPlayerController>> AllPlayerControllers;

	FTimerHandle RoundResetTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rule")
	float RoundResetDelay = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rule")
	float TurnTimeLimit = 20.0f;

	FTimerHandle TurnTickTimerHandle;

	bool bIsRoundActive;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rule")
	int32 MinPlayersToStart = 2;

	bool bIsGameStarted;

	UPROPERTY()
	TObjectPtr<ABBPlayerController> CurrentTurnPlayer;
};