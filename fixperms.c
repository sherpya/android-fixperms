#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <expat.h>


/* refs: frameworks/base/services/java/com/android/server/pm */

#define PACKAGES "packages.xml"
//#define PACKAGES "/data/system/packages.xml"

static char *blacklist[] =
{
    "framework-res.apk",
    "com.htc.resources.apk",
    NULL
};

typedef struct _APackage
{
    char name[PATH_MAX];
    char realName[PATH_MAX];
    char codePath[PATH_MAX];

    time_t ft;      /* timeStamp (hex) */
    time_t it;      /* firstInstallTime (hex) */
    time_t ut;      /* lastUpdateTime (hex) */

    int version;    /* versionCode */

    char resourcePath[PATH_MAX];
    char nativeLibraryPath[PATH_MAX];

    int userId;
    int sharedUserId;

    int flags;      /* pkgFlags */

    bool system;
    bool shared;
    bool skip;
} APackage;

static bool blCheck(const char *codePath)
{
    int i;
    for (i = 0; blacklist[i]; i++)
    {
        if (strstr(codePath, blacklist[i]))
            return true;
    }
    return false;
}

static void dumpPackage(APackage *package)
{
    printf("[%s] codePath:%s ", package->name, package->codePath);

    if (package->shared)
        printf("sharedUserId:%d ", package->sharedUserId);
    else
        printf("userId:%d ", package->userId);

    printf("version:%d ", package->version);
    printf("flags:%d ", package->flags);
    printf("\n");
}

#define TYPE_string(var) strncpy(package->var, attrs[i + 1], sizeof(package->var))
#define TYPE_time(var) package->var = strtoull(attrs[i + 1], NULL, 16)
#define TYPE_int(var) package->var = atoi(attrs[i + 1])

#define IFATTR(name, datatype) \
     if (!strcmp(attrs[i], #name)) \
        TYPE_##datatype(name);

static void startElement(void *userData, const XML_Char *name, const XML_Char **attrs)
{
    int i;
    APackage *package;

    if (strcmp(name, "package"))
        return;

    package = (APackage *) userData;
    memset(package, 0, sizeof(*package));
    package->userId = package->sharedUserId = -1;

    for (i = 0; attrs[i]; i += 2)
    {
        IFATTR(name, string)
        else IFATTR(realName, string)
        else IFATTR(codePath, string)
        else IFATTR(ft, time)
        else IFATTR(it, time)
        else IFATTR(ut, time)
        else IFATTR(version, int)
        else IFATTR(resourcePath, string)
        else IFATTR(nativeLibraryPath, string)
        else IFATTR(userId, int)
        else IFATTR(sharedUserId, int)
        else IFATTR(flags, int)

        package->skip = blCheck(package->codePath);
        package->shared = (package->sharedUserId != -1);
        package->system = !strncmp(package->codePath, "/system/app/", 12);
    }
}

static void endElement(void *userData, const XML_Char *name)
{
    if (strcmp(name, "package"))
        return;

    APackage *package = (APackage *) userData;
    dumpPackage(package);
}

int main(void)
{
    APackage package;
    char buffer[BUFSIZ];
    FILE *fd;
    size_t len;
    int res = 0;

    XML_Parser parser = XML_ParserCreate(NULL);

    if (!parser)
    {
        fprintf(stderr, "Memory allocation failed (ParserCreate)\n");
        return -1;
    }

    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetUserData(parser, &package);

    if (!(fd = fopen(PACKAGES, "r")))
    {
        perror("fopen");
        return -1;
    }

    
    while ((len = fread(buffer, 1, sizeof(buffer), fd)) > 0)
    {
        if (XML_Parse(parser, buffer, len, 0) == XML_STATUS_ERROR)
        {
            fprintf(stderr, "%s at line %lu\n", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
            res = -1;
            goto done;
        }
    }

done:
    fclose(fd);
    XML_Parse(parser, NULL, 0, 1);
    XML_ParserFree(parser);
    return res;

}
