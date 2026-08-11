#include "database.h"
#include "utf.h"
#include <stdlib.h>
#include <strsafe.h>

static void set_error(Database *db, wchar_t *error, size_t cap, const wchar_t *prefix) {
    wchar_t *detail = db && db->handle ? wide_from_utf8(sqlite3_errmsg(db->handle)) : NULL;
    StringCchPrintfW(error, cap, L"%s%s%s", prefix, detail ? L"\n" : L"", detail ? detail : L"");
    free(detail);
}

static BOOL exec_sql(Database *db, const char *sql, wchar_t *error, size_t cap) {
    if (sqlite3_exec(db->handle, sql, NULL, NULL, NULL) != SQLITE_OK) { set_error(db, error, cap, L"데이터베이스 초기화에 실패했습니다."); return FALSE; }
    return TRUE;
}

BOOL database_open(Database *db, const wchar_t *path, wchar_t *error, size_t cap) {
    char *utf8_path;
    if (!db) return FALSE;
    db->handle = NULL;
    utf8_path = utf8_from_wide(path);
    if (!utf8_path) { StringCchCopyW(error, cap, L"데이터베이스 경로 변환에 실패했습니다."); return FALSE; }
    if (sqlite3_open_v2(utf8_path, &db->handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) { free(utf8_path); set_error(db, error, cap, L"데이터베이스를 열 수 없습니다."); database_close(db); return FALSE; }
    free(utf8_path);
    sqlite3_busy_timeout(db->handle, 3000);
    if (!exec_sql(db, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;"
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, task_date TEXT NOT NULL, task_time TEXT,"
        "title TEXT NOT NULL, description TEXT, completed INTEGER NOT NULL DEFAULT 0,"
        "priority INTEGER NOT NULL DEFAULT 3, created_at TEXT NOT NULL, updated_at TEXT);", error, cap)) {
        database_close(db);
        return FALSE;
    }
    return TRUE;
}

void database_close(Database *db) { if (db && db->handle) { sqlite3_close(db->handle); db->handle = NULL; } }

static BOOL bind_text(sqlite3_stmt *s, int index, const wchar_t *value) {
    char *utf8 = utf8_from_wide(value); int rc;
    if (!utf8) return FALSE;
    rc = sqlite3_bind_text(s, index, utf8, -1, SQLITE_TRANSIENT); free(utf8);
    return rc == SQLITE_OK;
}

static BOOL bind_task(sqlite3_stmt *s, const Task *t) {
    if (!bind_text(s, 1, t->task_date)) return FALSE;
    if (t->task_time[0]) { if (!bind_text(s, 2, t->task_time)) return FALSE; } else sqlite3_bind_null(s, 2);
    return bind_text(s, 3, t->title) && bind_text(s, 4, t->description) &&
        sqlite3_bind_int(s, 5, t->completed) == SQLITE_OK && sqlite3_bind_int(s, 6, t->priority) == SQLITE_OK;
}

BOOL database_insert(Database *db, const Task *t, wchar_t *error, size_t cap) {
    sqlite3_stmt *s = NULL; BOOL ok = FALSE;
    const char *sql = "INSERT INTO tasks(task_date,task_time,title,description,completed,priority,created_at) VALUES(?,?,?,?,?,?,strftime('%Y-%m-%dT%H:%M:%f','now','localtime'));";
    if (sqlite3_prepare_v2(db->handle, sql, -1, &s, NULL) == SQLITE_OK && bind_task(s, t) && sqlite3_step(s) == SQLITE_DONE) ok = TRUE;
    if (!ok) set_error(db, error, cap, L"작업을 저장하지 못했습니다.");
    sqlite3_finalize(s); return ok;
}

BOOL database_update(Database *db, const Task *t, wchar_t *error, size_t cap) {
    sqlite3_stmt *s = NULL; BOOL ok = FALSE;
    const char *sql = "UPDATE tasks SET task_date=?,task_time=?,title=?,description=?,completed=?,priority=?,updated_at=strftime('%Y-%m-%dT%H:%M:%f','now','localtime') WHERE id=?;";
    if (sqlite3_prepare_v2(db->handle, sql, -1, &s, NULL) == SQLITE_OK && bind_task(s, t) && sqlite3_bind_int64(s, 7, t->id) == SQLITE_OK && sqlite3_step(s) == SQLITE_DONE) ok = sqlite3_changes(db->handle) == 1;
    if (!ok) set_error(db, error, cap, L"작업을 수정하지 못했습니다.");
    sqlite3_finalize(s); return ok;
}

BOOL database_delete(Database *db, int64_t id, wchar_t *error, size_t cap) {
    sqlite3_stmt *s = NULL; BOOL ok = FALSE;
    if (sqlite3_prepare_v2(db->handle, "DELETE FROM tasks WHERE id=?;", -1, &s, NULL) == SQLITE_OK && sqlite3_bind_int64(s, 1, id) == SQLITE_OK && sqlite3_step(s) == SQLITE_DONE) ok = sqlite3_changes(db->handle) == 1;
    if (!ok) set_error(db, error, cap, L"작업을 삭제하지 못했습니다.");
    sqlite3_finalize(s); return ok;
}

static void copy_column(sqlite3_stmt *s, int col, wchar_t *dest, size_t cap) {
    const unsigned char *value = sqlite3_column_text(s, col); wchar_t *wide;
    dest[0] = L'\0'; if (!value) return;
    wide = wide_from_utf8((const char *)value); if (wide) { StringCchCopyW(dest, cap, wide); free(wide); }
}

static void read_task(sqlite3_stmt *s, Task *t) {
    ZeroMemory(t, sizeof(*t)); t->id = sqlite3_column_int64(s, 0);
    copy_column(s, 1, t->task_date, 11); copy_column(s, 2, t->task_time, 6);
    copy_column(s, 3, t->title, TASK_TITLE_CAP); copy_column(s, 4, t->description, TASK_DESC_CAP);
    t->completed = sqlite3_column_int(s, 5); t->priority = sqlite3_column_int(s, 6);
}

BOOL database_get(Database *db, int64_t id, Task *t, wchar_t *error, size_t cap) {
    sqlite3_stmt *s = NULL; BOOL ok = FALSE;
    if (sqlite3_prepare_v2(db->handle, "SELECT id,task_date,task_time,title,description,completed,priority FROM tasks WHERE id=?;", -1, &s, NULL) == SQLITE_OK && sqlite3_bind_int64(s, 1, id) == SQLITE_OK && sqlite3_step(s) == SQLITE_ROW) { read_task(s, t); ok = TRUE; }
    if (!ok) set_error(db, error, cap, L"작업을 불러오지 못했습니다.");
    sqlite3_finalize(s); return ok;
}

BOOL database_list(Database *db, TaskList *list, wchar_t *error, size_t cap) {
    sqlite3_stmt *s = NULL; size_t count = 0, capacity = 16; Task *items = NULL; int rc;
    const char *sql = "SELECT id,task_date,task_time,title,description,completed,priority FROM tasks ORDER BY priority ASC,task_date ASC,CASE WHEN task_time IS NULL THEN 1 ELSE 0 END ASC,task_time ASC;";
    task_list_free(list);
    if (sqlite3_prepare_v2(db->handle, sql, -1, &s, NULL) != SQLITE_OK) { set_error(db, error, cap, L"작업 목록을 불러오지 못했습니다."); return FALSE; }
    items = (Task *)calloc(capacity, sizeof(Task));
    if (!items) { sqlite3_finalize(s); StringCchCopyW(error, cap, L"메모리가 부족합니다."); return FALSE; }
    while ((rc = sqlite3_step(s)) == SQLITE_ROW) {
        if (count == capacity) { Task *grown; capacity *= 2; grown = (Task *)realloc(items, capacity * sizeof(Task)); if (!grown) { free(items); sqlite3_finalize(s); StringCchCopyW(error, cap, L"메모리가 부족합니다."); return FALSE; } items = grown; }
        read_task(s, &items[count++]);
    }
    sqlite3_finalize(s);
    if (rc != SQLITE_DONE) { free(items); set_error(db, error, cap, L"작업 목록을 읽는 중 오류가 발생했습니다."); return FALSE; }
    list->items = items; list->count = count; return TRUE;
}
