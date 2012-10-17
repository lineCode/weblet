#include <plugin.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

weblet_host_interface_t host;
weblet_plugin_interface_s plugin =
{
    NULL,
};

const char *home_dir;

static int
cmpstringp(const void *p1, const void *p2)
{
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}

static size_t
complete_file_name(char ***result_ptr, char **path_prefix, const char *input_path, size_t size)
{
    char *p;
    char *path = strdup(input_path);
    
    if (path[0] != '/')
    {
        /* Relative Path : add HOME into path */
        size = asprintf(&p, "%s/%s", getenv("HOME"), path);
        free(path);
        path = p;
    }

    for (p = &path[size - 1]; p >= path; -- p)
    {
        if (*p == '/') break;
    }
        
    *p = 0;
    size_t basename_size = (path + size) - (p + 1);
    char *basename = strdup(p + 1);
    p = path;
    asprintf(&path, "%s/", path);
    free(p);
    DIR *dir = opendir(path);
    struct dirent *ent;
    char **cand;
    int count = 0, now;
    struct stat stat_buf;

    if (dir == NULL) goto failed;

    while ((ent = readdir(dir)) != NULL)
        if (strncmp(ent->d_name, basename, basename_size) == 0) count ++;

    cand = (char **)malloc(sizeof(char *) * count);
    if (cand == NULL)
    {
        closedir(dir);
        goto failed;
    }

    rewinddir(dir);
    now = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0) continue;
        if (strcmp(ent->d_name, "..") == 0) continue;
        if (strncmp(ent->d_name, basename, basename_size) == 0 && now < count)
        {
            char *testpath;
            struct stat stat_buf;
            asprintf(&testpath, "%s%s", path, ent->d_name);
            if (stat(testpath, &stat_buf) != 0)
            {
                free(testpath);
                continue;
            }
            else if (S_ISDIR(stat_buf.st_mode))
            {
                asprintf(&cand[now], "%s/", ent->d_name);
            }
            else cand[now] = strdup(ent->d_name);

            free(testpath);
            ++ now;
        }
    }

    cand = realloc(cand, sizeof(char *) * now);
    count = now;

    qsort(cand, count, sizeof(char *), cmpstringp);
    *result_ptr = cand;
    if (path_prefix)
        *path_prefix = path;
    else free(path);

    free(basename);
    closedir(dir);
    return count;

  failed:
    
    free(basename);
    free(path);
    return 0;
}

static char *
alloc_cstring_from_jsstring(JSStringRef js_str, size_t *size_ptr)
{
    size_t size = JSStringGetMaximumUTF8CStringSize(js_str);
    char *buf = (char *)malloc(size);
    if (buf)
        size = JSStringGetUTF8CString(js_str, buf, size);
    if (size_ptr) *size_ptr = size - 1;
    return buf;
}

static JSValueRef
js_cb_complete_file_name(JSContextRef context,
                         JSObjectRef function,
                         JSObjectRef self,
                         size_t argc,
                         const JSValueRef argv[],
                         JSValueRef* exception)
{
    if (argc == 1 && JSValueIsString(context, argv[0]))
    {
        JSStringRef string = JSValueToStringCopy(context, argv[0], NULL);
        size_t psize, csize;
        char *path = alloc_cstring_from_jsstring(string, &psize);
        JSStringRelease(string);

        char **comp, *path_prefix;
        csize = complete_file_name(&comp, &path_prefix, path, psize);
        free(path);

        if (csize == 0)
            return JSValueMakeNull(context);
        
        JSValueRef *arr = malloc(sizeof(JSValueRef *) * csize);
        int i;

        for (i = 0; i < csize; ++ i)
        {
            char *full;
            asprintf(&full, "%s%s", path_prefix, comp[i]);
            JSStringRef str = JSStringCreateWithUTF8CString(full);
            arr[i] = JSValueMakeString(context, str);
            JSStringRelease(str);
            free(full);
            free(comp[i]);
        }

        JSObjectRef obj = JSObjectMakeArray(context, csize, arr, NULL);

        for (i = 0; i < csize; ++ i)
            JSValueUnprotect(context, arr[i]);

        free(comp);
        free(arr);
        free(path_prefix);

        return (JSValueRef)obj;
    }
    
    return JSValueMakeNull(context);
}

weblet_plugin_interface_t
init(weblet_host_interface_t host_interface)
{
    host = host_interface;
    if ((home_dir = getenv("HOME")) == NULL) return NULL;
    
    host->register_native_method("CompleteFileName", js_cb_complete_file_name);
    
    return &plugin;
}
