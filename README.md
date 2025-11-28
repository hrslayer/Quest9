프로젝트 개요

엔진 버전: Unreal Engine 5.5
언어: C++
네트워크 모델: Server-Authoritative (서버 권한 기반)

-------------------------------------------------------------------------------------

핵심 기능:

데디케이티드 서버 지원 (접속 대기열, 인원 체크 후 게임 시작)

서버 권한의 턴 및 타이머 시스템

실시간 채팅 및 정답 판정 (Strike / Ball / Out)

클래스 구조 (Class Architecture)


1. ABBGameModeBase (Server Only)

접속 관리: InitNewPlayer로 닉네임을 파싱하고, OnPostLogin으로 접속 인원을 체크합니다.

게임 루프: 최소 인원(MinPlayersToStart)이 모이면 게임을 시작하고, 정답 숫자를 생성합니다.

턴 제어: TickTurnTimer를 통해 플레이어의 남은 시간을 관리하고, 시간 초과 시 패널티를 부여하거나 턴을 넘깁니다.

이탈 처리: Logout을 오버라이드하여 게임 도중 플레이어가 나갔을 때의 예외 처리를 담당합니다.


2. ABBPlayerController (Client & Server)

RPC 통신: ServerRPC_PrintChatting을 통해 클라이언트의 입력을 서버로 전송합니다.

UI 관리: BeginPlay 시점에 채팅 위젯(ChatInputWidget)과 정보 위젯(BBInfoWidget)을 생성하고 화면에 띄웁니다.

동기화: 서버로부터 받은 알림(AlarmText)을 UI에 전달합니다.


3. ABBPlayerState (Replicated)

데이터: PlayerNickName, CurrentNumberCount(시도 횟수), RemainingSeconds(남은 시간) 등을 관리합니다.

Replication: 모든 변수는 Replicated 설정되어 있어, 서버에서 값이 변하면 모든 클라이언트 UI가 즉시 갱신됩니다.


4. ABBGameStateBase

MulticastRPCLogin 함수를 통해 새로운 플레이어의 입장을 모든 클라이언트에게 공지합니다


5. UBBHUDWidget

정답 및 채팅 입력 (Enter 키 이벤트 처리).

NativeTick을 사용하여 실시간으로 남은 시간과 자신의 점수, 시스템 메시지를 갱신하여 보여줍니다.

-------------------------------------------------------------------------------------------

데이터 흐름 (Data Flow)


1. 접속 과정

Client 접속 시도 -> GameMode::InitNewPlayer (URL에서 이름 파싱) -> GameMode::OnPostLogin (인원 체크) -> (인원 충족 시) -> StartGame.


2. 게임 플레이

Client 입력 ("123") -> PlayerController -> ServerRPC -> GameMode 수신.

GameMode 판정 로직:

유효성 검사 (숫자 여부, 중복 여부).

정답 비교 (Strike/Ball).

결과 생성 ("1S 1B").

GameMode -> ClientRPC (모든 클라이언트에게 결과 전송) -> UI 출력.


3. 턴 & 타이머

GameMode의 타이머가 1초마다 PlayerState의 RemainingSeconds 감소.

Replication을 통해 Client의 BBInfoWidget이 시간 변화 감지 및 UI 갱신.

시간 초과 시 서버가 강제로 턴을 넘김.
