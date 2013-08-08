/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <utils/misc.h>
#include <utils/Trace.h>

namespace android {

volatile int32_t Tracer::sIsReady = 0;
int Tracer::sTraceFD = -1;
uint64_t Tracer::sEnabledTags = ATRACE_TAG_NOT_READY;
Mutex Tracer::sMutex;

void Tracer::changeCallback() {
    Mutex::Autolock lock(sMutex);
    if (sIsReady && sTraceFD >= 0) {
        loadSystemProperty();
    }
}

void Tracer::init() {
    Mutex::Autolock lock(sMutex);

    if (!sIsReady) {
        add_sysprop_change_callback(changeCallback, 0);

        const char* const traceFileName =
                "/sys/kernel/debug/tracing/trace_marker";
        sTraceFD = open(traceFileName, O_WRONLY);
        if (sTraceFD == -1) {
            ALOGW("error opening trace file: %s (%d)", strerror(errno), errno);
            sEnabledTags = 0;   // no tracing can occur
        } else {
            loadSystemProperty();
        }

        android_atomic_release_store(1, &sIsReady);
    }
}

static void traceInit() __attribute__((constructor));

static void traceInit()
{
    ::android::add_sysprop_change_callback(atrace_update_tags, 0);
}
