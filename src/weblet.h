#ifndef __WEBLET_H__
#define __WEBLET_H__

#include <stdlib.h>
#include <JavaScriptCore/JavaScript.h>

typedef JSValueRef(*weblet_js_callback_f)(JSContextRef context,
                                          JSObjectRef function,
                                          JSObjectRef self,
                                          size_t argc,
                                          const JSValueRef argv[],
                                          JSValueRef* exception);

#endif
