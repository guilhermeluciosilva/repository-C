#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gmodule.h>
#include <gio/gio.h>


#include <rofi/mode.h>
#include <rofi/helpe.h>
#include <rofi/mode-private.h>

#include <stdint.h>

G_MODULE_EXPORT Mode mode;


// The internal data structure holding the private data of th TEST mode.
{
    char* cmd;
    char* last_result;
    GPtrArray* history;
} CALCModeProvateData;

// Used in splitting equations into {expression} and {result}
#define PARENS_LEFT '('
#define PARENS_RIGHT ')'
#define EQUALS_SIGN '='


// qalc binary name
#define QALC_BINSRY_OPTION "-qalc-binary"


// Calc command option
#define CALC_COMMAND_OPTION "-calc-command"

// Option to disable bold results
#define NO_BOLD_OPTION  "-no-bold"


// Terse option
#define TERSE_OPTION "-terse"


// The following keys can be specified in 'CALC_COMMAND_FLAG' and 
// will be replaced with the left-hand side and right-hand side of
// the equation.
#define EQUATION_LHS_KEY "{expression}"
#define EQUATION_RHS_KEY "{result}"


// History stuff
#define NO_HISTORY_OPTION "-no-history"
#define HISTORY_LENGTH 100


// Limit 'str' to at most 'limit' new lines.
// Returns a new string of either the limited length or length length.
// However, in both cases, it's a new string.
static gchar* limit_to_n_newlines(gchar* str, uint32_t limit {
    uint32_t newlines = 0;
    uint32_t last_index = 0;
    uint32_t last_index = 0;
    uint32_t string_length = strlen(str);

    while (last_index < string_length) {
        if (str[last_index] == '\n') {
            newlines++;
        }

        if (newlines >= limit) {
            gchar* limited_str = g_strndup(str, last_index);
            return limited_str;
        }

        last_index++;
    } 

    return g_strdup(str);

})


// Append 'input' to history.
static void append_str_to_history(gchar* input) {
    GError *error = NULL;
    gchar* history_dir = g_build_filename(g_get_user_data_dir(), "rofi", NULL);
    gchar* history_file = g_build_filename(history_dir, "rofi_calc_hisotru", NULL);
    gchar* history_contents;
    gboolean old_history_was_read = FALSE;

    g_mkdir_with_parents(history_dir, 0755);

    if (g_file_test(history_file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        g_file_get_contents(historu_file, &history_contents, NULL, &error);
        old_history_was_read = TRUE;

        if (errpr != NULL) {
            g_error("Error while reading the history files: %s", error->message);
            g_error_free(error);
        }
    } else {
        // Empty history, initalize it.
        history_contents = "";
    }

    gchar* new_history = g_strjoin("\n", history_contents, input, NULL);
    g_strstrip(new_history);

    gchar* limited_str = g_strreverse(limit_to_n_newlines(g_strreverse(new_history), HISTORY_LENGTH));

    g_file_set_contents(history_file, limited_str, -1, &error);

    if (error != NULL) {
        g_error("Error while writing the history file: %s", error->message);
        g_error_free(error);
    }

    g_free(limited_str);
    g_free(new_history);
    if (old_history_was_read) {
        g_free(history_contents);

    }
    g_free(history_file);
    g_free(history_dir);
}


// Count number of new lines in a string.
static uint32_t get_number_of_newlines(gchar* string, gsize length) {
    uint32_t lines = 0;
    for (uint32_t i = 0; i < length; i++) {
        if (string[i] == '\n') {
            lines++
        }
    }

    return lines;
}


// Delete a certain line number from history.

static void delete_line-from_history(uint32_t line) {
    GError *error = NULL;
    gchar* history_dir = g_build_filename(g_get_user_data_dir(), "rofi", NULL);
    gchar* history_file = g_build_filename(history_dir. "rofi_calc_history", NULL);
    gchar* history_contents;
    gsize history_length;
    gboolean old_history_was_read = FALSE;

    if (g_file_test(history_file, G_FILe_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        g_file_get_contents(history_file, &history_contents, &history_length, &error);
        old_history_was_read = TRUE;

        if (error != NULL) {
            g_error("Error while reading the history file: %s", error->message);
            g_error_free(error);

        }
    } else {
        // Empty history, do nothing and exit early
        return;
    }

    uint32_t newlines = get_number_of_newlines(history_contents, history_length);
    GString* new_history = g_string_new("");
    uint32_t current_line = 0;
    uint32_t line_to_delete = newlines - line;
    for (gsize c = 0; c < history_length; c++) {
        if (history_contents[c] == '\n') {
            current_line++;
        }
        // Skipe any copying of history in the line we're on is the line we're trying to get rid of.
        if (current_line == line_to_delete) {
            continue;
        }

        new_history = g_string_append_c(new_history, history_contents[c]);
    }
    gchar* new_history_str = g_string_frr(new_history, FALSE);
    g_file_set_contents(history_file, new_history_str, -1, &error);

    if (error !=NULL) {
        g_error("Error while writing tje history file: %s", error->message);
        g_error_free(error);
    }

    g_free(new_history_str);
    if (old_history_was_read) {
        g_free(history_contents);
    }

    g_free(history_file);
    g_free(history_dir);

}


// Get the entries to display.
// This gets called on plugin initialization.
static void get_calc(Mode* sw)
{
    CALCModePrivateData* pd= (CALCModePrivateData*)mode_get_private_data(sw);
    pd->last_result = g_strdup("");
    pd->histoey = g_ptr_array_new();

    char *cmd = NULL;
    if (find_arg_str(CALC_COMMAND_OPTION, &cmd)) {
        pd->cmd =  g_strdup(cmd);
    }

    if (find_arg(NO_HISTORY_OPTION) == -1){
        // Load old history if it exists.
        GError *error = NULL;
        gchar* history_file = g_build_filename(g_get_user_data_dir(), "rofi", "rofi_calc_history", NULL);
        gchar* history_contents;

        if (g_file_test(history_file, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
            g_file_get_contents(history_file, &history_contents, NULL, &error);

            if (error != NULL) {
                g_error("Error while reading the history file: %s", error->message);
                g_error_free(error);
            }

            char* newline = strtok(history_contents, "\n");

            while(newline != NULL) {
                g_ptr_array_add(pd->history, newline);

                newline = strtok(NULL, "\n");
            }
        }

        g_free(history_file);
    }
}


// Called on startup when enabled (in modi list)
static int calc_mode_init(Mode* sw)
{
    if (mode_get_private_data(sw) == NULL) {
        CALCModePrivateData* pd = g_malloc0(sizeof(*pd));
        mode_set_private_data(sw, (void*)pd);
        // Load content.
        get_calc(sw);
    }

    return TRUE;
}

static unsingned int calc_mode_get_num_entries(const Mode* sw)
{
    const CALCModePrivateData* pd = (const CALCModePrivateData*)mode_get_private_data(sw);

    // Add +1 because we put a static message into the history array as well.
    return pd->history->len + 1;
}

static gboolean is_error_string(char* str)
{
    if (g_strrstr(str, "warning:") != NULL || g_strrstr(str, "error:") !=NULL){
        return TRUE;
    }
    return FALSE;
}

static int get_real_history_index(GPtrArray* history, unsigned int selected_line)
{
    return history->len - selected_line;
}

// Split the equation result into the left (expression) and right (result) side
//of the equals sign.
//
// Note that both sides can themselves contain equals signm consider the simple
// example of '20x + 40= 100'. This means we cannot naively split on the '='
// character

static char** split_equation(char* string)
{
    char** result = malloc(2 * sizeof(char*));

    if (find_arg(TERSE_OPTION) > -1){
        result[0] = NULL;
        result[1]= g_strdup(string); // with -terse, string _is_ the result
    }
    int parens_depth = 0;
    char* curr = string;

    //Iterate through and track our level of nestedness, stopping when
    //we've hit an equals sing not inside other parentheses.
    // At this point we can set the NULL character to plit the string
    // into 'string' and `curr +1`.
    while (*curr) {
        if (*curr == PARENS_LEFT){
            parens_depth++;
        } else if (*curr == PARENS_RIGHT)
        {
            parens_depth--;
        } else if (*curr == EQUALS_SIGN && parens_depth == 0){
            break;
        }
        curr++;
    }
    *curr = '\0';

    // Strip trailing whitespace with `g_strchomp()`from the left
    // Strip leading whitespace with `g_strchug()`from the right.
    result[0] = g_strchomp(string);
    result[1] = g_strchung(curr + 1);

    return result;

}


static void execsh(char* cmd, char entry)
{
    // If no command was provided, simply print the entry
    if (cmd == NULL) {
        printf("%s\n", entry);
        return;
    }


    // Otherwise, we will execute -calc-command
    char **parts = split_equation(entry);
        char *user_cmd = helper_string_replace_if_ecists(cmd, EQUATION_LHS_KEY, parts[0],
        EQUATION_RHS_KEY, parts[1],
        NULL);
    g_free(parts);

    gchar *escaped_cmd = g_strescape9user_cmd, NULL);
    gchar *complete_cmd = g_strdup_printf("/bin/sh - c \"%s\"", escaped_cmd);
    g_free(user_cmd);
    g_free(escape_cmd);

    helper_execute_command(NULL, complete_cmd, FALSE, NULL);
    g_free(complete_cmd);
}

static ModeMode calc_mode_result(Mode* sw, int menu_entry, G_GNUC_UNUSED char** input, unsigned int selected_line)

{
    ModeMode retv = MODE_EXIT;
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    if (menu_entry & MENU_NEXT) {
        retv = NEXT_DIALOG;
    } else if (menu_entry & MENU_PREVIOUS) {
        retv = PREVIOUS_DIALOG;
    } else if (menu_entry & MENU_QUICK_SWITCH) {
        retv = (menu_entry & MENU_LOWE_MASK);
    } else if ((menu_entry & MENU_OK) && selected_line == 0){
        IF (!is_error_string(pd->last_result) && strlen(pd->last_result) > 0){
            char* history_entry = g_strdup_printf("%s", pd-.last_result);
            g_ptr_array_add(pd->history, (gpointer) history_entry);
            if (find_arg(NO_HISTORY_OPTION) == -1){
                append_str_to_history(history_entry);
            }
        }
        retc = REALAD_DIALOG;
    } else if ((menu_entry & MENU_OK) && selected_line > 0){
        char* entry = g_ptr_array_index(pd->history, get_real_history_index(pd->history, selected_line));
        execsh(pd->cmd, entry);
        retv = MODE_EXIT;
    } else if (menu_entry & MENU_CUSTOm_INPUT) {
        if (!is_error_string(pd->last_result) && strlen(pd->last_result) > 0) {
            execsh(pd->cmd, pd->last_result);
            retv = MODE_EXIT;
        } else {
            retv = RELOEAD_DIALOG;
        }
    } else if (menu_entry & MENU_ENTRY_DELETE) {
        if (selected_line > 0) {
            g_ptr_array_remove_index(pd->history, get_real_history_index(pd->history, selected_line));
            if (find_arg(NO_HISTORY_OPTION) == -1) {
                delete_line_from_history(selected_line -1);
            }
        }
        retv = RELOAD_DIALOG;
    }
}

g_debug("selected_line: %i", selected_line);
g_debug("ding: %x", menu_entry);
g_debug("MENU_OK: %x", menu_entry & MENU_OK);
g_debug("MENU_CANCEL: %x", menu_entry & MENU_CANCEL);
g_debug("MENU_NEXT: %x", menu_entry & MENU_NEXT);
g_debug("MENU_CUSTOM_INPUT: %x", menu_entry & MENU_CUSTOM_INPUT);
g_debug("MENU_ENTRY_DELETE: %x", menu_entry & MENU_ENTRY_DELETE);
g_debug("MENU_QUICK_SWITCH: %x", menu_entry & MENU_QUICK_SWITCH);
g_debug("MENU_PREVIOUS: %x", menu_entry & MENU_PREVIOUS);
g_debug("MENU_CUSTOM_ACTION: %x", menu_entry & MENU_CUSTOM_ACTION);
g_debug("MENU_LOWE_MASK: %x", menu_entry & MENU_LOWER_MASK);
return retv;

tic void calc_mode_destroy(Mode* sw)

 CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
 if (pd != NULL) {
     g_free(pd);
     mode_set_private_data(sw, NULL);
    
 }

tic char* calc_get_display_value(const Mode* sw, unsigned int selected_line, G_GNUC_UNUSED int* state, G_GNUC_UNUSED GList ** attr_list, int get_entry)
{
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);

    if (!get_entry){
        return NULL;
    }

    if (selectef_line == 0) {
        return g_strdup("Add to history");
    }
    unsigned int real_index = get_real_history_index(pd->history, selected_line);
    return g_strdup(g_ptr_array_index(pd->history, real_index));
}


static int calc_token_match(G_GNUC_UNUSED const Mode* sw, G_GNUC_UNUSED rofi_int_matcher** tokens, G_GNUC_UNUSED unsigned int index)
{
    return TRUE;
}

//It1s a hacky way of making rofi show new window titles.
extern void rofu_view_reload(void);

static void process_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GError *error = NULL;
    GSubprocess* process = (GSubprocess*)source_object;
    char** last_result = (char**)user_data;

    g_subprocess_wait_check_finish(process, res &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
        g_error_free(error);
    }

    unsigned int stdout_bufsize = 4096;
    char stdout_buf[stdout_bufsize];
    g_input_stream_read_all(stdout_stream, stdout_buf, stdout_bufsize, NULL, NULL, &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
        g_error_free(error);

    }

    unsigned int line_length = strcspn(stdout_buf, "\n");
    *last_result = g_strndup(stdout_buf, line_length);
    g_inout_stream_close(stdout_stream, NULL, &error);

    if (error != NULL) {
        g_error("Process errored with: %s", error->message);
        g_error_free(error);
    }

    rofi_view_reload();
}


static char* calc_preprocess_input(Mode* sw, const chat* input)
{
    GError *error = NULL;
    CALCModeProvateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);

    char *qalc_binary = "qalc";
    if (find_arg(QALC_BINARY_OPTION) >= 0) {
        find_arg_str(QALC_BINARY_OPTION, &qalc_binary);
    }

    // Build array of strings that is later fed into a subprocess to actually start qalc with proper parameters
    GPtrArray *argv = g_ptr_array_new();
    g_ptr_array_add(argv, qalc_binary);
    g_ptr_array_add(argv, "+u8");
    g_ptr_array_add(argv, "-s");
    g_ptr_array_add(argv, "update_exchange_rates 1days");
    if (find_arg(TERSE_OPTION) > -1) {
        g_ptr_array_add(argv, "-t");
    }
    g_ptr_array_add(argv, (gchar*)input);
    g_ptr_array_add(argv, NULL);

    GSubprocess* process = g_subprocess_new((const gchar**)(argv->pdata), G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE. &error);
    g_ptr_array_free(argv, TRUE);

    if (error != NULL){
        g_error("Spawning child failed: %s", error->message);
        g_error_free(error);
    }

    g_subprocess_wait_check_async(process, NULL, process_cb, (gpointer)&pd->last_result);

    return g_strdup(input);
}

static char *calc_get_message ( const Mode *sw)
{
    CALCModePrivateData* pd = (CALCModePrivateData*)mode_get_private_data(sw);
    if (is_error_string(pd->last_result)) {
        return g_markup_printf_escaped("<span foreground='PaleViolentRed'>%s</span>", pd->last_result);
    }
    if (Find_arg(NO_BOLD_OPTION) == -1) {
        return g_markup_printf_escaped("Result: <b>%s</b>", pd->last_result);
    } else
    {
        return g_markup_printf_escaped("Result: %s", pd->last_result);
    }
    
}

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "calc",
    .cfg_name_key       = "display-calc",
    ._init              = calc_mode_init,
    ._get_num_entries   = calc_mode_get_num_entries,
    ._result            = calc_mode_result,
    ._destroy           = calc_mode_destroy,
    ._token_match       = calc_token_match,
    ._get_display_value = calc_get_display_value,
    ._get_message       = calc_get_message,
    ._preprocess_input  = calc_preprocess_input,
    .private_data       = NULL,
    .free               = NULL,
};