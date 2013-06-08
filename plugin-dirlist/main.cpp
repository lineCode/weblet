#include <plugin.h>
#include <spawn.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "lib.hpp"

#define CACHE_HEAD "PLUGIN_DIRLIST_CACHE"

using namespace std;

weblet_host_interface_t host;
weblet_plugin_interface_s plugin =
{
    NULL,
};

string cache_path_prefix;

static int
read_utf8_string(string &s, FILE *f) {
    ostringstream oss;
    oss.str("");
    while (!feof(f)) {
        int c = fgetc(f);
        if (c < 0) return 1;
        if (c == 0) break;
        oss << (char)c;
    }
    s = oss.str();
    return 0;
}


static int
dirlist(vector<string> &r, const string &dirname) {
    int cached = 0;
    string edirname;
    string cachename;
    ostringstream oss;
    
    encode_path(edirname, dirname);
    oss << cache_path_prefix << edirname;
    cachename = oss.str();

    time_t dir_time, cache_time;
    struct stat statbuf;
    
    if (stat(dirname.c_str(), &statbuf)) goto failed;
    if (!S_ISDIR(statbuf.st_mode)) goto failed;
    dir_time = statbuf.st_mtime;

    if (stat(cachename.c_str(), &statbuf) == 0) {
        // Assume the cache file is hold by plugin
        cache_time = statbuf.st_mtime;
        if (dir_time <= cache_time) cached = 1;
    }

  again:

    r.clear();

    if (cached) {
        // read the cache
        FILE *f = fopen(cachename.c_str(), "r");
        if (f == NULL) { cached = 0; goto again; }
        while (!feof(f)) {
            string s;
            if (read_utf8_string(s, f))
            {
                if (feof(f)) break;
                else { fclose(f); cached = 0; goto again; }
            }
            r.push_back(s);
        }
        fclose(f);
    } else {
        fprintf(stderr, "Building cache for %s\n", dirname.c_str());
        // build the cache
        DIR *dir = opendir(dirname.c_str());
        if (dir == NULL) goto failed;
        struct dirent *ent;
    
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0) continue;
            if (strcmp(ent->d_name, "..") == 0) continue;

            r.push_back(ent->d_name);
        }

        // Using '\0' as the delimeter. We are using UTF-8 so no problem! +_+
        FILE *f = fopen(cachename.c_str(), "w");
        if (f != NULL) {
            fputs(CACHE_HEAD, f);
            fputc(0, f);
            for (int i = 0; i < r.size(); ++ i)
            {
                size_t s = r[i].length();
                fwrite(r[i].c_str(), s, 1, f);
                fputc(0, f);
            }
            fclose(f);
        
            // set time stamp
            struct utimbuf times;
            times.actime = dir_time;
            times.modtime = dir_time;
            utime(cachename.c_str(), &times);
        }
        else
        {
            fprintf(stderr, "Cannot open file %s as the dir list cache\n", cachename.c_str());
        }
    }

    return 0;

  failed:
    return 1;
}

static JSValueRef
js_cb_dir_list(JSContextRef context,
               JSObjectRef function,
               JSObjectRef self,
               size_t argc,
               const JSValueRef argv[],
               JSValueRef* exception)
{
    if (argc != 1)
        return JSValueMakeNull(context);
    if (!JSValueIsString(context, argv[0]))
        return JSValueMakeNull(context);

    JSStringRef js_dirname = JSValueToStringCopy(context, argv[0], NULL);
    string dirname;
    int r =  copy_from_jsstring(dirname, js_dirname);
    JSStringRelease(js_dirname);
    if (r) return JSValueMakeNull(context);

    vector<string> rlist;
    r = dirlist(rlist, dirname);
    if (r || rlist.size() == 0) goto failed;
    
    JSValueRef *arr;
    arr = (JSValueRef *)malloc(sizeof(JSValueRef) * rlist.size());
    if (arr == NULL) goto failed;
    
    for (int i = 0; i < rlist.size(); ++ i)
    {
        JSStringRef str = JSStringCreateWithUTF8CString(rlist[i].c_str());
        arr[i] = JSValueMakeString(context, str);
        JSStringRelease(str);
    }

    JSObjectRef obj;
    obj = JSObjectMakeArray(context, rlist.size(), arr, NULL);

    for (int i = 0; i < rlist.size(); ++ i)
        JSValueUnprotect(context, arr[i]);

    free(arr);
    return (JSValueRef)obj;
    
  failed:
    return JSValueMakeNull(context);
}

static bool
cmpString(const string &a, const string &b)
{
    return strcmp(a.c_str(), b.c_str()) < 0;
}

static JSValueRef
js_cb_list_prog(JSContextRef context,
                JSObjectRef function,
                JSObjectRef self,
                size_t argc,
                const JSValueRef argv[],
                JSValueRef* exception)
{
    if (argc == 1 && JSValueIsString(context, argv[0]))
    {
        JSStringRef js_name = JSValueToStringCopy(context, argv[0], NULL);
        string name;
        int r;
        r = copy_from_jsstring(name, js_name);
        JSStringRelease(js_name);
        
        if (r) return JSValueMakeNull(context);
        
        char *paths = strdup(getenv("PATH"));
        char *dir = paths;
        char *nextdir;

        vector<string> comp_all, comp_prefix, comp_contain;
        vector<string> comp;
        
        while (dir != NULL)
        {
            nextdir = strchr(dir, ':');
            if (nextdir)
            {
                *(nextdir ++) = 0;
            }

            int r = dirlist(comp, dir);
            if (r == 0)
            {
                for (int i = 0; i < comp.size(); ++ i) {
                    ostringstream oss;
                    oss << dir << "/" << comp[i];
                    string filename = oss.str();
                    struct stat statbuf;
                    if (stat(filename.c_str(), &statbuf)) continue;
                    if (!S_ISREG(statbuf.st_mode)) continue;
                    if (!(statbuf.st_mode & 0111)) continue;
                    // a regular and executable item now

                    size_t idx = comp[i].find(name);
                    if (idx == 0)
                        comp_prefix.push_back(comp[i]);
                    else if (idx != string::npos)
                        comp_contain.push_back(comp[i]);
                }
            }

            dir = nextdir;
        }        

        sort(comp_prefix.begin(), comp_prefix.end(), cmpString);
        sort(comp_contain.begin(), comp_contain.end(), cmpString);

        // put the suggestion as first element
        if (comp_prefix.size() == 0) {
            if (comp_contain.size() == 1)
                comp_all.push_back(comp_contain[0]);
            else comp_all.push_back(name);
        } else {
            ostringstream oss;
            int i = 0;
            string &u = comp_prefix[0]; 
            string &v = comp_prefix[comp_prefix.size() - 1];
            while (i < u.length() && i < v.length() && u[i] == v[i]) {
                oss << u[i];
                ++ i;
            }
            comp_all.push_back(oss.str());
        }


        comp_all.insert(comp_all.end(), comp_prefix.begin(), comp_prefix.end());
        comp_all.insert(comp_all.end(), comp_contain.begin(), comp_contain.end());

        if (comp_all.size() == 0)
            return JSValueMakeNull(context);

        JSValueRef *arr = (JSValueRef *)malloc(sizeof(JSValueRef) * comp_all.size());
        
        int i;
        for (i = 0; i < comp_all.size(); ++ i)
        {
            JSStringRef str = JSStringCreateWithUTF8CString(comp_all[i].c_str());
            arr[i] = JSValueMakeString(context, str);
            JSStringRelease(str);
        }

        JSObjectRef obj = JSObjectMakeArray(context, comp_all.size(), arr, NULL);

        for (i = 0; i < comp_all.size(); ++ i)
            JSValueUnprotect(context, arr[i]);

        free(arr);
        return (JSValueRef)obj;
    }
    
    return JSValueMakeNull(context);
}

extern "C" {
    
weblet_plugin_interface_t
init(weblet_host_interface_t host_interface)
{
    ostringstream oss;
    oss << getenv("WEBLET_PATH") << "/cache/";
    cache_path_prefix = oss.str();
    
    host = host_interface;
    host->register_native_method("ListDir", js_cb_dir_list);
    host->register_native_method("ListProg", js_cb_list_prog);
    return &plugin;
}

}
