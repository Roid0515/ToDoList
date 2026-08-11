#include "ui_internal.h"

#define UI_MARGIN 12
#define UI_LABEL_WIDTH 72
#define UI_DATE_Y 10
#define UI_TITLE_Y 43
#define UI_DESCRIPTION_Y 75
#define UI_PRIORITY_Y 155
#define UI_ACTION_BUTTON_Y 191
#define UI_TASK_LIST_Y 232
#define UI_FOOTER_HEIGHT 45

void ui_layout_reset_task_columns(AppState *app)
{
    RECT client;
    int fixed_width;
    int title_width;

    GetClientRect(app->task_list, &client);
    fixed_width = g_task_columns[0].width +
        g_task_columns[1].width +
        g_task_columns[2].width +
        g_task_columns[4].width;
    title_width = (client.right - client.left) - fixed_width;

    if (title_width < 80) {
        title_width = 80;
    }

    ListView_SetColumnWidth(app->task_list, 0, g_task_columns[0].width);
    ListView_SetColumnWidth(app->task_list, 1, g_task_columns[1].width);
    ListView_SetColumnWidth(app->task_list, 2, g_task_columns[2].width);
    ListView_SetColumnWidth(app->task_list, 3, title_width);
    ListView_SetColumnWidth(app->task_list, 4, g_task_columns[4].width);
}

void ui_layout_controls(AppState *app, int width, int height)
{
    int content_width = width - UI_MARGIN * 2;
    int field_x = UI_MARGIN + UI_LABEL_WIDTH;
    int field_width = content_width - UI_LABEL_WIDTH;
    int button_width = (content_width - 18) / 4;
    int footer_y = height - UI_FOOTER_HEIGHT;

    MoveWindow(app->date_label, UI_MARGIN, 14, UI_LABEL_WIDTH, 24, TRUE);
    MoveWindow(app->title_label, UI_MARGIN, 46, UI_LABEL_WIDTH, 24, TRUE);
    MoveWindow(app->desc_label, UI_MARGIN, 78, UI_LABEL_WIDTH, 24, TRUE);
    MoveWindow(app->priority_label, UI_MARGIN, 158, UI_LABEL_WIDTH, 24, TRUE);

    MoveWindow(app->date_picker, field_x, UI_DATE_Y, 130, 26, TRUE);
    MoveWindow(app->time_check, field_x + 140, UI_DATE_Y + 1, 88, 24, TRUE);
    MoveWindow(app->time_picker, field_x + 230, UI_DATE_Y, 92, 26, TRUE);
    MoveWindow(app->title_edit, field_x, UI_TITLE_Y, field_width, 26, TRUE);
    MoveWindow(
        app->desc_edit,
        field_x,
        UI_DESCRIPTION_Y,
        field_width,
        74,
        TRUE);
    MoveWindow(app->priority_combo, field_x, UI_PRIORITY_Y, 115, 200, TRUE);
    MoveWindow(app->completed_check, field_x + 130, UI_PRIORITY_Y + 2, 70, 24, TRUE);

    MoveWindow(
        app->add_button,
        UI_MARGIN,
        UI_ACTION_BUTTON_Y,
        button_width,
        30,
        TRUE);
    MoveWindow(
        app->update_button,
        UI_MARGIN + button_width + 6,
        UI_ACTION_BUTTON_Y,
        button_width,
        30,
        TRUE);
    MoveWindow(
        app->delete_button,
        UI_MARGIN + (button_width + 6) * 2,
        UI_ACTION_BUTTON_Y,
        button_width,
        30,
        TRUE);
    MoveWindow(
        app->clear_button,
        UI_MARGIN + (button_width + 6) * 3,
        UI_ACTION_BUTTON_Y,
        button_width,
        30,
        TRUE);

    MoveWindow(
        app->task_list,
        UI_MARGIN,
        UI_TASK_LIST_Y,
        content_width,
        footer_y - UI_TASK_LIST_Y - 8,
        TRUE);
    ui_layout_reset_task_columns(app);

    MoveWindow(app->topmost_check, UI_MARGIN, footer_y + 8, 80, 26, TRUE);
    MoveWindow(app->opacity_label, UI_MARGIN + 92, footer_y + 10, 92, 22, TRUE);
    MoveWindow(
        app->opacity_trackbar,
        UI_MARGIN + 180,
        footer_y + 5,
        content_width - 180,
        34,
        TRUE);
}
