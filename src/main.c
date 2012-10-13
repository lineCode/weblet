#include <stdlib.h>
#include <stdio.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>

#include <unistd.h>

#include "weblet.h"

#define NATIVE_OBJECT_NAME "sys"

GtkWindow  *mainWindow = NULL;
WebKitWebView *webView = NULL;

JSGlobalContextRef jsNativeContext;
JSObjectRef        jsNativeObject;

static void destroy_cb(GtkWidget *widget, gpointer data) { mainWindow = NULL; gtk_main_quit(); }

static void
load_status_cb(WebKitWebFrame *frame,
               gboolean arg1,
               gpointer data)
{
    if (webkit_web_frame_get_load_status(frame) == WEBKIT_LOAD_COMMITTED)
    {
        JSStringRef name;
        name = JSStringCreateWithUTF8CString(NATIVE_OBJECT_NAME);
        JSGlobalContextRef js_global_context =
            webkit_web_frame_get_global_context(frame);
        
        JSObjectSetProperty(js_global_context,
                            JSContextGetGlobalObject(js_global_context),
                            name,
                            *(JSObjectRef *)data,
                            0, NULL);
        JSStringRelease(name);
    }
}

int
register_native_method(const char *method_name, weblet_js_callback_f func)
{
    JSStringRef name = JSStringCreateWithUTF8CString(method_name);
    JSObjectSetProperty(jsNativeContext,
                        jsNativeObject,
                        name,
                        JSObjectMakeFunctionWithCallback(
                            jsNativeContext, NULL,
                            func),
                        0, NULL);
    JSStringRelease(name);
}


static JSValueRef
js_cb_say_hello(JSContextRef context,
                JSObjectRef function,
                JSObjectRef self,
                size_t argc,
                const JSValueRef argv[],
                JSValueRef* exception)
{
    /* GtkAllocation area; */
    /* area.x = area.y = 0; */
    /* area.width = area.height = 0; */
    /* gtk_widget_size_allocate(GTK_WIDGET(webView), &area); */
    return JSValueMakeNull(context);
}

static JSValueRef
js_cb_get_desktop_focus(JSContextRef context,
                        JSObjectRef function,
                        JSObjectRef self,
                        size_t argc,
                        const JSValueRef argv[],
                        JSValueRef* exception)
{
    gtk_window_present(mainWindow);
    return JSValueMakeNull(context);
}

#define CMD_LINE_SIZE  1024
#define CMD_ARGS_SIZE 256

static JSValueRef
js_cb_launcher_submit(JSContextRef context,
                      JSObjectRef function,
                      JSObjectRef self,
                      size_t argc,
                      const JSValueRef argv[],
                      JSValueRef* exception)
{
    if (argc != 2)
        return JSValueMakeNull(context);

    int len = JSValueToNumber(context, argv[0], NULL);
    JSObjectRef arr = JSValueToObject(context, argv[1], NULL);

    char  cmd_str_buf[CMD_LINE_SIZE];
    int   cmd_str_buf_cur = 0;
    char *cmd_idx_buf[CMD_ARGS_SIZE + 2];

    if (len >= CMD_ARGS_SIZE) return JSValueMakeNull(context);

    int i;
    for (i = 0; i < len; ++ i)
    {
        JSValueRef cur = JSObjectGetPropertyAtIndex(context, arr, i, NULL);
        JSStringRef str = JSValueToStringCopy(context, cur, NULL);
        size_t l = JSStringGetUTF8CString(str, cmd_str_buf + cmd_str_buf_cur, CMD_LINE_SIZE - cmd_str_buf_cur);
        cmd_idx_buf[i + 2] = cmd_str_buf + cmd_str_buf_cur;
        cmd_str_buf_cur += l;
        JSStringRelease(str);
        JSValueUnprotect(context, cur);
    }
    cmd_idx_buf[0] = "/bin/sh";
    cmd_idx_buf[1] = "-c";
    cmd_idx_buf[i + 2] = 0;

    if (fork() == 0)
    {
        execvp(cmd_idx_buf[0], cmd_idx_buf);
        fprintf(stderr, "execv returns, too bad.\n");
        exit(-1);
    }

    return JSValueMakeNull(context);
}


int
main(int argc, char* argv[]) {    
    gtk_init(&argc, &argv);

    if(!g_thread_supported())
        g_thread_init(NULL);

    // Create a Window, set visual to RGBA
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_widget_set_can_focus(window, TRUE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
    
    if (visual == NULL || !gtk_widget_is_composited(window))
    {
        fprintf(stderr, "Your screen does not support alpha window, :(\n");
        visual = gdk_screen_get_system_visual(screen);
    }

    gtk_widget_set_visual(GTK_WIDGET(window), visual);
    
    GdkRGBA bg = {1,1,1,0};
    gtk_widget_override_background_color(GTK_WIDGET(window), GTK_STATE_FLAG_NORMAL, &bg);
    g_signal_connect(window, "destroy", G_CALLBACK(destroy_cb), NULL);

    // Create a WebView, set it transparent, add it to the window
    WebKitWebView *web_view = web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    webkit_web_view_set_transparent(web_view, TRUE);
    /* WebKitWebSettings *web_setting = webkit_web_view_get_settings(web_view); */
    /* g_object_set(web_setting, */
    /*              "auto-resize-window", TRUE, */
    /*              NULL); */
    
    /* webkit_web_view_set_settings(web_view, web_setting); */
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));

    /* Create the native object with local callback properties */
    
    JSGlobalContextRef js_global_context =
        webkit_web_frame_get_global_context(webkit_web_view_get_main_frame(web_view));

    JSStringRef native_object_name;
    jsNativeContext = JSGlobalContextCreateInGroup(
        JSContextGetGroup(
            webkit_web_frame_get_global_context(
                webkit_web_view_get_main_frame(web_view))),
        NULL);
    jsNativeObject = JSObjectMake(jsNativeContext, NULL, NULL);
    native_object_name = JSStringCreateWithUTF8CString(NATIVE_OBJECT_NAME);
    JSObjectSetProperty(jsNativeContext,
                        JSContextGetGlobalObject(jsNativeContext),
                        native_object_name,
                        jsNativeObject,
                        0, NULL);
    JSStringRelease(native_object_name);
    
    g_signal_connect(webkit_web_view_get_main_frame(web_view), "notify::load-status",
                     G_CALLBACK(load_status_cb), &jsNativeObject);
    
    // Show it and continue running until the window closes
    gtk_widget_grab_focus(GTK_WIDGET(web_view));

    mainWindow = GTK_WINDOW(window);
    webView = web_view;

    // Example of registering JS native call
    register_native_method("SayHello", js_cb_say_hello);
    register_native_method("GetDesktopFocus", js_cb_get_desktop_focus);
    register_native_method("LauncherSubmit", js_cb_launcher_submit);

    // Load a default page
    char *cwd = g_get_current_dir();
    char *path = g_build_filename(cwd, getenv("WEBLET_HOME"), NULL);
    char *start = g_filename_to_uri(path, NULL, NULL);

    webkit_web_view_load_uri(web_view, start);

    g_free(cwd);
    g_free(path);
    g_free(start);

    gtk_widget_show_all(GTK_WIDGET(mainWindow));

    gtk_main();
    
    return 0;
}
