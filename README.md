# Personal ToDo

Windows 11용 경량 개인 ToDo List MVP입니다. C17, Win32 API, Windows Common Controls, SQLite C API만 사용합니다.

## 빌드

Visual Studio 2022 또는 Build Tools의 x64 Native Tools 환경에서:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

실행 파일은 `build/Release/TodoList.exe`에 생성됩니다. 외부 DLL, Python, .NET Runtime, 데이터베이스 서버가 필요하지 않습니다.

배포용 V2 실행 파일은 `release/TodoList_V2.exe`입니다.

## 사용법

- 날짜, 선택적 시간, 제목, 내용, 우선순위, 완료 여부를 입력한 뒤 **등록**을 누릅니다.
- 기존 작업을 수정하거나 삭제하려면 목록의 행을 더블 클릭합니다.
- 아래 슬라이더로 창 투명도를 30~100% 범위에서 조절합니다.
- **항상 위**를 켜면 다른 창 위에 유지됩니다.

## 사용자 데이터

프로그램 데이터는 실행 파일과 분리되어 다음 위치에 저장됩니다.

```text
%LOCALAPPDATA%\PersonalTodo\
├─ data\todo.db
├─ config\settings.ini
└─ logs\app.log
```

백업하려면 프로그램을 종료한 뒤 `todo.db` 파일을 복사하십시오.

## 데이터베이스 스키마

```sql
CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    task_date TEXT NOT NULL,
    task_time TEXT,
    title TEXT NOT NULL,
    description TEXT,
    completed INTEGER NOT NULL DEFAULT 0,
    priority INTEGER NOT NULL DEFAULT 3,
    created_at TEXT NOT NULL,
    updated_at TEXT
);
```

목록은 우선순위, 날짜, 지정 시간 순서로 정렬되며 시간이 없는 작업은 같은 날짜의 시간 지정 작업 뒤에 표시됩니다.

## MVP 범위

구현: 로컬 데이터 폴더/DB 자동 생성, 작업 CRUD, 영구 저장, 우선순위 정렬, 선택적 시간, 완료 상태, 창 크기/위치/투명도/항상 위 설정 저장, 화면 밖 위치 복구, 1MB 로그 순환.

제외: 시스템 트레이, 자동 시작, 반복 작업, 검색/필터, 알림, 클라우드 동기화, 테마.

프로그램은 단일 GUI 스레드와 이벤트 기반 메시지 루프를 사용합니다. 백그라운드 스레드, 타이머, 폴링이 없어 유휴 상태에서 지속적인 DB 조회가 발생하지 않습니다.

## UI 모듈 구조

- `ui_main.c`: 창 생명주기와 Win32 메시지 라우팅
- `ui_controls.c`: 컨트롤 생성, 컬럼 정의, 폼 초기화
- `ui_layout.c`: 컨트롤 배치와 ListView 컬럼 너비
- `ui_tasks.c`: 작업 폼 처리, 목록 갱신, CRUD 연결
- `ui_settings.c`: 투명도, 항상 위, 창 상태 저장
- `ui_internal.h`: UI 모듈 사이의 내부 계약

V1.5의 기존 단일 파일 구현은 비교와 변경 이력 확인을 위해 `ui.c`에 비활성 코드로 보존되어 있으며 빌드에는 포함되지 않습니다.
