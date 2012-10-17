#include <plugin.h>
#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

weblet_host_interface_t host;
weblet_plugin_interface_s plugin =
{
    NULL,
};

extern char **environ;

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

    static const int CMD_LINE_SIZE = 4096;
    static const int CMD_ARGS_SIZE = 256;

    char  cmd_str_buf[CMD_LINE_SIZE];
    int   cmd_str_buf_cur = 0;
    char *cmd_idx_buf[CMD_ARGS_SIZE];

    if (len == 0 || len >= CMD_ARGS_SIZE) return JSValueMakeNull(context);

    int i;
    for (i = 0; i < len; ++ i)
    {
        JSValueRef cur = JSObjectGetPropertyAtIndex(context, arr, i, NULL);
        JSStringRef str = JSValueToStringCopy(context, cur, NULL);
        size_t l = JSStringGetUTF8CString(str, cmd_str_buf + cmd_str_buf_cur, CMD_LINE_SIZE - cmd_str_buf_cur);
        cmd_idx_buf[i] = cmd_str_buf + cmd_str_buf_cur;
        cmd_str_buf_cur += l;
        JSStringRelease(str);
        JSValueUnprotect(context, cur);
    }
    cmd_idx_buf[i] = 0;

    pid_t pid;
    if (posix_spawnp(&pid, cmd_idx_buf[0], NULL, NULL, cmd_idx_buf, environ) == 0)
    { }
    else
    {
        fprintf(stderr, "posix spawn failed, too bad.\n");
    }

    return JSValueMakeNull(context);
}

weblet_plugin_interface_t
init(weblet_host_interface_t host_interface)
{
    host = host_interface;

    host->register_native_method("LauncherSubmit", js_cb_launcher_submit);
    
    return &plugin;
}
