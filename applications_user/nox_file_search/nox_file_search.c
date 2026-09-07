/* File Search - recursively find files on the SD card by name.
 *
 * Shipped as a .fap rather than as a scene inside the Archive app on purpose. Archive's context
 * menu is capped at MENU_ITEMS (5) and its 48px menu box is already full in the default
 * file-manager branch, so a sixth entry would have broken the menu geometry. As an external app
 * this costs zero firmware flash, which matters on this board, and it cannot regress the file
 * manager.
 */

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/loading.h>

#include <storage/storage.h>
#include <toolbox/dir_walk.h>

#define TAG "FileSearch"

#define SEARCH_ROOT      "/ext"
#define QUERY_LEN        32
#define MAX_RESULTS      64
#define RESULT_LABEL_LEN 34

typedef enum {
    ViewIdQuery,
    ViewIdLoading,
    ViewIdResults,
    ViewIdDetail,
} ViewId;

typedef enum {
    // Start at 1: 0 is a poor choice for a custom event id.
    EventSearchDone = 1,
} AppEvent;

typedef struct {
    Gui* gui;
    Storage* storage;
    ViewDispatcher* view_dispatcher;
    TextInput* text_input;
    Loading* loading;
    Submenu* submenu;
    Widget* widget;

    FuriThread* worker;
    char query[QUERY_LEN];

    FuriString* results[MAX_RESULTS];
    size_t result_count;
    bool truncated;
    char header[32];
} FileSearch;

/* ---- helpers ---- */

static char lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
}

// Case-insensitive substring test. strcasestr is not available here.
static bool contains_ci(const char* haystack, const char* needle) {
    if(*needle == '\0') return true;
    for(const char* h = haystack; *h; h++) {
        const char* a = h;
        const char* b = needle;
        while(*a && *b && lower(*a) == lower(*b)) {
            a++;
            b++;
        }
        if(*b == '\0') return true;
    }
    return false;
}

static const char* basename_of(const char* path) {
    const char* slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static void results_clear(FileSearch* app) {
    for(size_t i = 0; i < app->result_count; i++) {
        furi_string_free(app->results[i]);
        app->results[i] = NULL;
    }
    app->result_count = 0;
    app->truncated = false;
}

/* ---- worker ---- */

static int32_t search_worker(void* context) {
    FileSearch* app = context;

    DirWalk* dir_walk = dir_walk_alloc(app->storage);
    dir_walk_set_recursive(dir_walk, true);

    FuriString* path = furi_string_alloc();

    if(dir_walk_open(dir_walk, SEARCH_ROOT)) {
        FileInfo info;
        while(app->result_count < MAX_RESULTS) {
            const DirWalkResult res = dir_walk_read(dir_walk, path, &info);
            if(res != DirWalkOK) {
                // DirWalkLast ends the walk normally; DirWalkError also stops it.
                break;
            }
            if(file_info_is_dir(&info)) continue;

            const char* full = furi_string_get_cstr(path);
            if(contains_ci(basename_of(full), app->query)) {
                app->results[app->result_count++] = furi_string_alloc_set(path);
            }
        }
        // Distinguish "hit the cap" from "walked everything".
        if(app->result_count >= MAX_RESULTS) {
            app->truncated = true;
        }
    } else {
        FURI_LOG_W(TAG, "Cannot open " SEARCH_ROOT " - is the SD card mounted?");
    }

    furi_string_free(path);
    dir_walk_free(dir_walk);

    view_dispatcher_send_custom_event(app->view_dispatcher, EventSearchDone);
    return 0;
}

/* ---- callbacks ---- */

static void query_submitted(void* context) {
    FileSearch* app = context;

    results_clear(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdLoading);

    app->worker = furi_thread_alloc_ex("FileSearchWorker", 4096, search_worker, app);
    furi_thread_start(app->worker);
}

static void result_selected(void* context, uint32_t index) {
    FileSearch* app = context;
    if(index >= app->result_count) return;

    widget_reset(app->widget);
    widget_add_text_scroll_element(
        app->widget, 0, 0, 128, 64, furi_string_get_cstr(app->results[index]));
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdDetail);
}

static bool custom_event(void* context, uint32_t event) {
    FileSearch* app = context;

    if(event != EventSearchDone) return false;

    furi_thread_join(app->worker);
    furi_thread_free(app->worker);
    app->worker = NULL;

    submenu_reset(app->submenu);

    if(app->result_count == 0) {
        submenu_set_header(app->submenu, "No matches");
    } else if(app->truncated) {
        snprintf(app->header, sizeof(app->header), "First %d matches", MAX_RESULTS);
        submenu_set_header(app->submenu, app->header);
    } else {
        submenu_set_header(app->submenu, "Matches");
    }

    for(size_t i = 0; i < app->result_count; i++) {
        // Submenu keeps only the pointer, so label storage must outlive it: point at the
        // basename inside the result string, which lives until results_clear().
        const char* full = furi_string_get_cstr(app->results[i]);
        submenu_add_item(app->submenu, basename_of(full), i, result_selected, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdResults);
    return true;
}

static uint32_t back_to_query(void* context) {
    UNUSED(context);
    return ViewIdQuery;
}

static uint32_t back_to_results(void* context) {
    UNUSED(context);
    return ViewIdResults;
}

static uint32_t exit_app(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

/* ---- app ---- */

int32_t nox_file_search_app(void* p) {
    UNUSED(p);

    FileSearch* app = malloc(sizeof(FileSearch));
    memset(app, 0, sizeof(FileSearch));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, custom_event);

    app->text_input = text_input_alloc();
    text_input_set_header_text(app->text_input, "Find file (name contains)");
    text_input_set_result_callback(
        app->text_input, query_submitted, app, app->query, sizeof(app->query), true);
    view_set_previous_callback(text_input_get_view(app->text_input), exit_app);

    app->loading = loading_alloc();

    app->submenu = submenu_alloc();
    view_set_previous_callback(submenu_get_view(app->submenu), back_to_query);

    app->widget = widget_alloc();
    view_set_previous_callback(widget_get_view(app->widget), back_to_results);

    view_dispatcher_add_view(
        app->view_dispatcher, ViewIdQuery, text_input_get_view(app->text_input));
    view_dispatcher_add_view(app->view_dispatcher, ViewIdLoading, loading_get_view(app->loading));
    view_dispatcher_add_view(app->view_dispatcher, ViewIdResults, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->view_dispatcher, ViewIdDetail, widget_get_view(app->widget));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIdQuery);

    view_dispatcher_run(app->view_dispatcher);

    // The worker always ends by posting EventSearchDone, which joins it. If the user exits while
    // it is still running, join here so nothing is freed under it.
    if(app->worker) {
        furi_thread_join(app->worker);
        furi_thread_free(app->worker);
    }

    view_dispatcher_remove_view(app->view_dispatcher, ViewIdQuery);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIdLoading);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIdResults);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIdDetail);
    view_dispatcher_free(app->view_dispatcher);

    widget_free(app->widget);
    submenu_free(app->submenu);
    loading_free(app->loading);
    text_input_free(app->text_input);

    results_clear(app);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
