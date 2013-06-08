#ifndef __PLUGIN_DIRLIST_CACHE_LIB__
#define __PLUGIN_DIRLIST_CACHE_LIB__

#include <plugin.h>
#include <string>

int  copy_from_jsstring(std::string &result, JSStringRef js_str);
void encode_path(std::string &result, const std::string &path);
void decode_path(std::string &result, const std::string &epath);

#endif
