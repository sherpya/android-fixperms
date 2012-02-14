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

#include "fixperms.h"
#include "hashmap.h"

#ifdef __ANDROID__
#define PACKAGES "/data/system/packages.xml"
#else
#define PACKAGES "packages.xml"
#endif

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
