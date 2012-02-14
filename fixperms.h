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

#ifndef _FIXPERMS_H
#define _FIXPERMS_H

#include <limits.h>
#include <stdbool.h>
#include "hashmap.h"

typedef struct _APK
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
} APK;

/* packages.c */
extern Hashmap *readPackages(const char *filename);
extern bool dumpPackage(void *key, void *value, void *context);
extern bool freePackage(void *key, void *value, void *context);
extern bool blCheck(const char *codePath);

/* perms.c */
extern bool initUsers(void);
extern bool checkPackage(void *key, void *value, void *context);
#endif /* _FIXPERMS_H */
