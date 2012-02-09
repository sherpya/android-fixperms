#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <expat.h>

#define PACKAGES "packages.xml"

static char *blacklist[] =
{
    "framework-res.apk",
    "com.htc.resources.apk",
    NULL
};

typedef struct _APackage
{
    char name[PATH_MAX];
    int userId;
    int sharedUserId;
    int shared;
    int skip;
} APackage;

static int blCheck(const char *codePath)
{
    int i;
    for (i = 0; blacklist[i]; i++)
    {
        if (strstr(codePath, blacklist[i]))
            return 1;
    }
    return 0;
}

static void startElement(void *userData, const XML_Char *name, const XML_Char **attrs)
{
    int i;
    APackage *package;

    if (strcmp(name, "package"))
        return;

    package = (APackage *) userData;
    memset(package, 0, sizeof(*package));

    for (i = 0; attrs[i]; i += 2)
    {
        if (!strcmp(attrs[i], "name"))
            strncpy(package->name, attrs[i + 1], sizeof(package->name));
        else if (!strcmp(attrs[i], "codePath"))
            package->skip = blCheck(attrs[i + 1]);
        else if (!strcmp(attrs[i], "userId"))
            package->userId = atoi(attrs[i + 1]);
        else if (!strcmp(attrs[i], "sharedUserId"))
        {
            package->sharedUserId = atoi(attrs[i + 1]);
            package->shared = 1;
        }
    }
}

static void endElement(void *userData, const XML_Char *name)
{
    if (strcmp(name, "package"))
        return;

    APackage *package = (APackage *) userData;
    if (package->skip)
    {
        printf("Skipping %s\n", package->name);
        return;
    }
    printf("%s - userId: %d - sharedUserId: %d - is shared %d\n", package->name, package->userId, package->sharedUserId, package->shared);
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
