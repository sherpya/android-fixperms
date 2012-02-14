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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include "fixperms.h"

#ifdef __ANDROID__
#define SYSTEMUSER "system"
#else
#define SYSTEMUSER "root"
#endif

static uid_t sysuid;
static gid_t sysgid;
static uid_t appuid;
static gid_t appgid;

const static uid_t rootuid = 0;
const static gid_t rootgid = 0;

typedef struct _permission
{
    const char *prefix;
    const uid_t *uid;
    const gid_t *gid;
    const mode_t mode;
} permission;

static const permission permissions [] =
{
        { "/data/app/",         &sysuid,    &sysgid,    0644 },
        { "/data/app-private/", &sysuid,    &appuid,    0640 },
        { "/system/app/",       &rootuid,   &rootgid,   0644 },
        { "/system/framework/", &rootuid,   &rootgid,   0644 },
        { "/mnt/asec/",         &sysuid,    &rootgid,   0555 }
};

#define PERMISSION_MAX sizeof(permissions) / sizeof(permission)

bool initUsers(void)
{
    struct passwd *sysuser = getpwnam(SYSTEMUSER);
    if (!sysuser)
    {
        fprintf(stderr, "Cannot find system user\n");
        return false;
    }
    sysuid = sysuser->pw_uid;
    sysgid = sysuser->pw_gid;
    return true;
}

bool checkCodePath(APK *package)
{
    struct stat info;
    bool result = true;

    /* codePath */
    if (stat(package->codePath, &info) < 0)
    {
        perror("stat");
        return false;
    }

    mode_t mode = info.st_mode & 07777;

    const permission *perm = NULL;
    int i;
    for (i = 0; i < PERMISSION_MAX; i++)
    {
        if (strstr(package->codePath, permissions[i].prefix) == package->codePath)
        {
            perm = &permissions[i];
            break;
        }
    }

    if (!perm)
    {
        fprintf(stderr, "Sorry, I don't known how to handle codePath: %s\n", package->codePath);
        return false;
    }

    printf("codePath %s uid %lu/%lu - perm %o:", package->codePath, info.st_uid, info.st_gid, mode);

    if ((info.st_uid != *perm->uid) || (info.st_gid != *perm->gid))
    {
        printf(" Wrong uid/gid (should be %d/%d)", *perm->uid, *perm->gid);
        result = false;
    }

    if (mode != perm->mode)
    {
        printf(" Wrong permissions (should be %o)", perm->mode);
        result = false;
    }

    if (result)
        printf(" OK\n");
    else
        printf("\n");
    return result;
}

bool checkPackage(void *key, void *value, void *context)
{
    APK *package = (APK *) value;
    printf("[%s] ", package->name);
    checkCodePath(package);
    return true;
}
