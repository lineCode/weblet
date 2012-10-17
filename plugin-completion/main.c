#include <plugin.h>

weblet_host_interface_t host;
weblet_plugin_interface_s plugin =
{
    NULL,
};

static JSValueRef
js_cb_complete_file_name(JSContextRef context,
                         JSObjectRef function,
                         JSObjectRef self,
                         size_t argc,
                         const JSValueRef argv[],
                         JSValueRef* exception)
{
    return JSValueMakeNull(context);
}

weblet_plugin_interface_t
init(weblet_host_interface_t host_interface)
{
    host = host_interface;

    host->register_native_method("CompleteFileName", js_cb_complete_file_name);
    
    return &plugin;
}
