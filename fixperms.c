/*
 * Copyright (c) 2012 Gianluigi Tiesi <sherpya@netfarm.it>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
  refs: frameworks/base/services/java/com/android/server/pm
  idea: fix_permissions (cyanogen & others)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <expat.h>

#include "fixperms.h"
#include "hashmap.h"

#define PACKAGES "packages.xml"
//#define PACKAGES "/data/system/packages.xml"

/* system/core/libcutils/str_parms.c */
static bool str_eq(void *key_a, void *key_b)
{
    return !strcmp((const char *) key_a, (const char *) key_b);
}

/* use djb hash unless we find it inadequate */
static int str_hash_fn(void *str)
{
    uint32_t hash = 5381;
    char *p;

    for (p = str; p && *p; p++)
        hash = ((hash << 5) + hash) + *p;
    return (int) hash;
}

static bool blCheck(const char *codePath)
{
    static const char *blacklist[] =
    {
        "framework-res.apk",
        "com.htc.resources.apk",
        NULL
    };

    int i;
    for (i = 0; blacklist[i]; i++)
    {
        if (strstr(codePath, blacklist[i]))
            return true;
    }
    return false;
}

static bool dumpPackage(void *key, void *value, void *context)
{
    APK *package = (APK *) value;
    printf("[%s] codePath:%s ", package->name, package->codePath);

    if (package->shared)
        printf("sharedUserId:%d ", package->sharedUserId);
    else
        printf("userId:%d ", package->userId);

    printf("version:%d ", package->version);
    printf("flags:%d ", package->flags);
    printf("\n");
    return true;
}

static bool freePackage(void *key, void *value, void *context)
{
    free(value);
    return true;
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
    APK *package;
    Hashmap *packages;
    void *oldval;

    if (strcmp(name, "package"))
        return;

    package = (APK *) calloc(1, sizeof(APK));
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
    }

    package->shared = (package->sharedUserId != -1);
    package->system = !strncmp(package->codePath, "/system/app/", 12);

    packages = (Hashmap *) userData;
    if ((oldval = hashmapPut(packages, package->name, package)))
        free(oldval);
}

static void endElement(void *userData, const XML_Char *name)
{
}

Hashmap *readPackages(const char *filename)
{
    Hashmap *packages = NULL;
    char buffer[BUFSIZ];
    size_t len;
    FILE *fd;

    XML_Parser parser = XML_ParserCreate(NULL);

    if (!parser)
    {
        fprintf(stderr, "Memory allocation failed (XML_ParserCreate)\n");
        return NULL;
    }

    packages = hashmapCreate(500, str_hash_fn, str_eq);

    if (!packages)
    {
        fprintf(stderr, "Memory allocation failed (hashmapCreate)\n");
        return NULL;
    }

    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetUserData(parser, packages);


    if (!(fd = fopen(PACKAGES, "r")))
    {
        perror("fopen");
        hashmapFree(packages);
        return NULL;
    }
    
    while ((len = fread(buffer, 1, sizeof(buffer), fd)) > 0)
    {
        if (XML_Parse(parser, buffer, len, 0) == XML_STATUS_ERROR)
        {
            fprintf(stderr, "%s at line %lu\n", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
            hashmapForEach(packages, freePackage, NULL);
            hashmapFree(packages);
            packages = NULL;
            goto done;
        }
    }

done:
    fclose(fd);
    XML_Parse(parser, NULL, 0, 1);
    XML_ParserFree(parser);

    return packages;
}

int main(int argc, char *argv[])
{
    Hashmap *packages;

    if (!(packages = readPackages(PACKAGES)))
        return -1;

    hashmapForEach(packages, dumpPackage, NULL);
    printf("\nPackages: %u\n", (int) hashmapSize(packages));

    if (argc == 2)
    {
        APK *package = (APK *) hashmapGet(packages, argv[1]);
        if (package)
            dumpPackage(NULL, package, NULL);
        else
            printf("Package %s not found\n", argv[1]);
    }

    hashmapForEach(packages, freePackage, NULL);
    hashmapFree(packages);
    return 0;
}
