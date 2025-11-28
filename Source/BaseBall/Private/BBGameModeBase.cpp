// Fill out your copyright notice in the Description page of Project Settings.


#include "BBGameModeBase.h"
#include "BBGameStateBase.h"
#include "BBPlayerController.h"
#include "BBPlayerState.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ABBGameModeBase::ABBGameModeBase()
{
	bIsGameStarted = false;
	bIsRoundActive = false;
}

void ABBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	bIsGameStarted = false;
	bIsRoundActive = false;
}

FString ABBGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	FString PlayerName = FString::Printf(TEXT("Player%d"), GetNumPlayers());

	ABBPlayerState* BBPS = NewPlayerController->GetPlayerState<ABBPlayerState>();
	if (IsValid(BBPS))
	{
		BBPS->PlayerNickName = PlayerName;
	}

	return Result;
}

void ABBGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABBPlayerController* BBPC = Cast<ABBPlayerController>(NewPlayer);
	if (IsValid(BBPC))
	{
		AllPlayerControllers.Add(BBPC);

		ABBPlayerState* BBPS = BBPC->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS))
		{
			if (BBPS->PlayerNickName == TEXT("None") || BBPS->PlayerNickName.IsEmpty())
			{
				BBPS->PlayerNickName = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
			}

			if (!bIsGameStarted)
			{
				FString WaitMsg = FString::Printf(TEXT("Waiting for players... (%d/%d)"), AllPlayerControllers.Num(), MinPlayersToStart);
				BBPC->InfoText = FText::FromString(WaitMsg);
			}
			else
			{
				BBPC->InfoText = FText::FromString(TEXT("Game is currently in progress."));
			}
		}

		ABBGameStateBase* BBGS = GetGameState<ABBGameStateBase>();
		if (IsValid(BBGS))
		{
			BBGS->MulticastRPCLogin(BBPS->PlayerNickName);
		}

		if (!bIsGameStarted && AllPlayerControllers.Num() >= MinPlayersToStart)
		{
			StartGame();
		}
	}
}

void ABBGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABBPlayerController* BBPC = Cast<ABBPlayerController>(Exiting);
	if (IsValid(BBPC))
	{
		AllPlayerControllers.Remove(BBPC);

		if (bIsGameStarted && CurrentTurnPlayer == BBPC)
		{
			if (AllPlayerControllers.Num() < MinPlayersToStart)
			{
				bIsGameStarted = false;
				bIsRoundActive = false;
				GetWorldTimerManager().ClearTimer(TurnTickTimerHandle);

				for (ABBPlayerController* PC : AllPlayerControllers)
				{
					if (IsValid(PC)) PC->InfoText = FText::FromString(TEXT("Player Disconnected. Game Stopped."));
				}
			}
			else
			{
				ResetGame();
			}
		}
	}
}

void ABBGameModeBase::StartGame()
{
	bIsGameStarted = true;

	for (ABBPlayerController* PC : AllPlayerControllers)
	{
		if (IsValid(PC)) PC->InfoText = FText::FromString(TEXT("Game Start!"));
	}

	ResetGame();
}

void ABBGameModeBase::PrintChattingMessage(ABBPlayerController* ChattingPlayerController, const FString& InChattingMessage)
{
	if (!bIsGameStarted) return;

	if (IsValid(CurrentTurnPlayer) && ChattingPlayerController != CurrentTurnPlayer)
	{
		ChattingPlayerController->InfoText = FText::FromString(TEXT("It's not your turn!"));
		return;
	}

	FString ChattingMessages = InChattingMessage;
	int Index = InChattingMessage.Len() - 3;

	if (Index < 0)
	{
		for (ABBPlayerController* BBPC : AllPlayerControllers)
		{
			if (IsValid(BBPC))
			{
				BBPC->ClientRPC_PrintChatting(InChattingMessage);
			}
		}
		return;
	}

	FString RightAnswerNumber = InChattingMessage.RightChop(Index);
	if (IsValidGuessNumber(RightAnswerNumber))
	{
		int32 StrikeCount = 0;
		FString MakeGuessResultString = MakeGuessResult(NumberRightAnswer, RightAnswerNumber, StrikeCount);

		UpGuessCount(ChattingPlayerController);
		JudgeGame(ChattingPlayerController, StrikeCount);

		for (ABBPlayerController* BBPC : AllPlayerControllers)
		{
			if (IsValid(BBPC))
			{
				FString ConnetionMessage = InChattingMessage + TEXT(" -> ") + MakeGuessResultString;
				BBPC->ClientRPC_PrintChatting(ConnetionMessage);
			}
		}
	}
	else
	{
		for (ABBPlayerController* BBPC : AllPlayerControllers)
		{
			if (IsValid(BBPC))
			{
				BBPC->ClientRPC_PrintChatting(InChattingMessage);
			}
		}
	}
}

FString ABBGameModeBase::MakeSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) {return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ABBGameModeBase::IsValidGuessNumber(const FString& InGuessNumberString)
{
	bool bCanPlay = false;

	do
	{
		if (InGuessNumberString.Len() != 3) break;

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InGuessNumberString)
		{
			if (!FChar::IsDigit(C) || C == '0')
			{
				bIsUnique = false;
				break;
			}
			UniqueDigits.Add(C);
		}

		if (!bIsUnique) break;

		bCanPlay = true;
	} while (false);

	return bCanPlay;
}

FString ABBGameModeBase::MakeGuessResult(const FString& SecretNumberString, const FString& GuessNumberString, int32& OutStrikeCount)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (SecretNumberString[i] == GuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), GuessNumberString[i]);
			if (SecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	OutStrikeCount = StrikeCount;

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABBGameModeBase::UpGuessCount(ABBPlayerController* InChattingPlayerController)
{
	if (!IsValid(InChattingPlayerController)) return;

	ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
	if (!IsValid(BBPS)) return;

	BBPS->CurrentNumberCount++;
	BBPS->RemainingSeconds = 0;

	const int32 CurrentPlayerCount = AllPlayerControllers.Num();
	if (CurrentPlayerCount <= 0) return;

	if (CurrentPlayerCount == 1)
	{
		BBPS->RemainingSeconds = static_cast<int32>(TurnTimeLimit);
		CurrentTurnPlayer = InChattingPlayerController;
	}
	else
	{
		const int32 CurrentIndex = AllPlayerControllers.IndexOfByKey(InChattingPlayerController);
		if (CurrentIndex != INDEX_NONE)
		{
			const int32 NextIndex = (CurrentIndex + 1) % CurrentPlayerCount;
			ABBPlayerController* NextPC = AllPlayerControllers[NextIndex];
			if (IsValid(NextPC))
			{
				ABBPlayerState* NextPS = NextPC->GetPlayerState<ABBPlayerState>();
				if (IsValid(NextPS))
				{
					NextPS->RemainingSeconds = static_cast<int32>(TurnTimeLimit);
					CurrentTurnPlayer = NextPC;
				}
			}
		}
	}

	UpdateAllPlayersTimer();
}

void ABBGameModeBase::ResetGame()
{
	if (AllPlayerControllers.Num() < MinPlayersToStart)
	{
		bIsGameStarted = false;
		bIsRoundActive = false;
		GetWorldTimerManager().ClearTimer(TurnTickTimerHandle);
		return;
	}

	NumberRightAnswer = MakeSecretNumber();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *NumberRightAnswer);

	bIsRoundActive = true;
	CurrentTurnPlayer = nullptr;

	for (ABBPlayerController* BBPC : AllPlayerControllers)
	{
		if (IsValid(BBPC))
		{
			ABBPlayerState* BBPS = BBPC->GetPlayerState<ABBPlayerState>();
			if (IsValid(BBPS))
			{
				BBPS->CurrentNumberCount = 0;
				BBPS->RemainingSeconds = 0;
			}
			BBPC->InfoText = FText::FromString(TEXT("New Round!"));
		}
	}

	if (AllPlayerControllers.Num() > 0)
	{
		CurrentTurnPlayer = AllPlayerControllers[0];
		ABBPlayerState* PS = CurrentTurnPlayer->GetPlayerState<ABBPlayerState>();
		if (PS)
		{
			PS->RemainingSeconds = static_cast<int32>(TurnTimeLimit);
		}

		if (!GetWorldTimerManager().IsTimerActive(TurnTickTimerHandle))
		{
			GetWorldTimerManager().SetTimer(TurnTickTimerHandle, this, &ABBGameModeBase::TickTurnTimer, 1.0f, true);
		}
	}
}

void ABBGameModeBase::JudgeGame(ABBPlayerController* InChattingPlayerController, int StrikeCount)
{
	if (StrikeCount == 3)
	{
		ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();

		if (IsValid(BBPS))
		{
			FString ConnetionMessage = BBPS->PlayerNickName + TEXT(" You Win!");

			for (ABBPlayerController* BBPC : AllPlayerControllers)
			{
				if (IsValid(BBPC))
				{
					BBPC->InfoText = FText::FromString(ConnetionMessage);
				}
			}
			bIsRoundActive = false;
			CurrentTurnPlayer = nullptr;

			GetWorldTimerManager().SetTimer(RoundResetTimerHandle, this, &ABBGameModeBase::ResetGame, RoundResetDelay, false);
		}
	}
	else
	{
		ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();

		if (IsValid(BBPS))
		{
			if (BBPS->CurrentNumberCount >= BBPS->MaxNumberCount)
			{
				for (ABBPlayerController* BBPC : AllPlayerControllers)
				{
					if (IsValid(BBPC))
					{
						BBPC->InfoText = FText::FromString(TEXT("Draw..."));
					}
				}
				bIsRoundActive = false;
				CurrentTurnPlayer = nullptr;

				GetWorldTimerManager().SetTimer(RoundResetTimerHandle, this, &ABBGameModeBase::ResetGame, RoundResetDelay, false);
			}
		}
	}
}

void ABBGameModeBase::TickTurnTimer()
{
	if (!bIsGameStarted || !bIsRoundActive) return;
	if (AllPlayerControllers.Num() == 0) return;

	if (!IsValid(CurrentTurnPlayer))
	{
		ResetGame();
		return;
	}

	for (ABBPlayerController* BBPC : AllPlayerControllers)
	{
		if (!IsValid(BBPC)) continue;
		if (BBPC != CurrentTurnPlayer) continue;

		ABBPlayerState* BBPS = BBPC->GetPlayerState<ABBPlayerState>();
		if (!IsValid(BBPS)) continue;

		if (BBPS->RemainingSeconds > 0)
		{
			BBPS->RemainingSeconds--;

			if (BBPS->RemainingSeconds <= 0)
			{
				BBPS->CurrentNumberCount++;

				FString TimeOverMessage = BBPS->PlayerNickName + TEXT(" Time Over!");
				for (ABBPlayerController* PC : AllPlayerControllers)
				{
					if (IsValid(PC)) PC->InfoText = FText::FromString(TimeOverMessage);
				}

				int32 DummyStrikeCount = 0;
				JudgeGame(CurrentTurnPlayer, DummyStrikeCount);

				if (bIsRoundActive)
				{
					const int32 CurrentPlayerCount = AllPlayerControllers.Num();
					if (CurrentPlayerCount > 0)
					{
						const int32 CurrentIndex = AllPlayerControllers.IndexOfByKey(CurrentTurnPlayer);
						if (CurrentIndex != INDEX_NONE)
						{
							const int32 NextIndex = (CurrentIndex + 1) % CurrentPlayerCount;
							ABBPlayerController* NextPC = AllPlayerControllers[NextIndex];
							if (IsValid(NextPC))
							{
								ABBPlayerState* NextPS = NextPC->GetPlayerState<ABBPlayerState>();
								if (IsValid(NextPS))
								{
									NextPS->RemainingSeconds = static_cast<int32>(TurnTimeLimit);
									CurrentTurnPlayer = NextPC;
								}
							}
						}
					}
				}
			}
		}
	}

	UpdateAllPlayersTimer();
}

void ABBGameModeBase::UpdateAllPlayersTimer()
{
	FString CombinedMessage;

	for (ABBPlayerController* BBPC : AllPlayerControllers)
	{
		if (!IsValid(BBPC)) continue;

		ABBPlayerState* BBPS = BBPC->GetPlayerState<ABBPlayerState>();
		if (!IsValid(BBPS)) continue;
		if (BBPS->RemainingSeconds <= 0) continue;

		FString Line = FString::Printf(TEXT("%s (%d / %d) - %d sec"), *BBPS->PlayerNickName, BBPS->CurrentNumberCount, BBPS->MaxNumberCount, BBPS->RemainingSeconds);
		if (!CombinedMessage.IsEmpty())
		{
			CombinedMessage.Append(TEXT("\n"));
		}
		CombinedMessage.Append(Line);
	}

	if (CombinedMessage.IsEmpty()) return;

	for (ABBPlayerController* BBPC : AllPlayerControllers)
	{
		if (IsValid(BBPC))
		{
			BBPC->InfoText = FText::FromString(CombinedMessage);
		}
	}
}