#include "lib.hpp"
#include <sstream>

using namespace std;

int
copy_from_jsstring(string &result, JSStringRef js_str)
{
    size_t size = JSStringGetMaximumUTF8CStringSize(js_str);
    char *buf = (char *)malloc(size);
    if (buf)
        size = JSStringGetUTF8CString(js_str, buf, size);
    else return 1;
    result = string(buf, size - 1);
    free(buf);
    return 0;
}

void
encode_path(string &result, const string &path) {
    ostringstream oss;
    for (int i = 0; i < path.length(); ++ i) {
        if (path[i] != '/' && path[i] != '_') {
            oss << path[i];
        }
        else if (path[i] != '/' || i != path.length() - 1)
        {
            oss << '_' << (path[i] == '/' ? '1' : '2');
        }
    }
    result = oss.str();
}

void
decode_path(string &result, const string &epath) {
    ostringstream oss;
    int e = 0;
    for (int i = 0; i < epath.length(); ++ i) {
        if (epath[i] == '_')
            e = 1;
        else if (e) {
            oss << (epath[i] == '1' ? '/' : '_');
            e = 0;
        } else oss << epath[i];
    }
    result = oss.str();
}
