#ifndef __WEBLET_PLUGIN_H__
#define __WEBLET_PLUGIN_H__

#include <stdlib.h>
#include <JavaScriptCore/JavaScript.h>

typedef JSValueRef(*weblet_js_callback_f)(JSContextRef context,
                                          JSObjectRef function,
                                          JSObjectRef self,
                                          size_t argc,
                                          const JSValueRef argv[],
                                          JSValueRef* exception);

typedef struct weblet_plugin_interface_s *weblet_plugin_interface_t;
typedef struct weblet_plugin_interface_s
{
    void(*exit)(weblet_plugin_interface_t);
} weblet_plugin_interface_s;

typedef struct weblet_host_interface_s *weblet_host_interface_t;
typedef struct weblet_host_interface_s
{
    int(*const register_native_method)(const char *, weblet_js_callback_f);
} weblet_host_interface_s;

typedef weblet_plugin_interface_t(*weblet_plugin_init_f)(weblet_host_interface_t);

#endif
