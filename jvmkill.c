/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <deque>
#include <mutex>

using namespace std;

#include "jvmkill.h"

enum {
    TIME_OPT = 0,
    COUNT_OPT,
    THE_END
};
 
char *tokens[] = {
    [TIME_OPT] = strdup("time"),
    [COUNT_OPT] = strdup("count"),
    [THE_END] = NULL
};

static std::mutex re_mutex;
static deque<long> events; 
static struct Configuration configuration;

void setSignal(int signal) {
   configuration.signal = signal;
}
long getTimeMillis() {
   struct timeval  tv;
   gettimeofday(&tv, NULL);
   return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
}

static void pruneEvents() {
   long millisLimit = getTimeMillis()-configuration.time_threshold*1000;
   for (int i=events.size()-1;i>0;i--) {
     long value = events[i]; 
     if (value >= millisLimit)
        break;
     else
        events.pop_back();
   }
}
void
resourceExhausted(
      jvmtiEnv *jvmti_env,
      JNIEnv *jni_env,
      jint flags,
      const void *reserved,
      const char *description) {
   fprintf(stderr, "ResourceExhausted!\n");
   std::lock_guard<std::mutex> lock(re_mutex);

   pruneEvents();
   long event = getTimeMillis();
   events.push_front(event);
   int eventCount = events.size();
   if (eventCount > configuration.count_threshold) {
   	kill(getpid(), configuration.signal);
        fprintf(stderr, "killing current process\n");
   }
}

int setCallbacks(jvmtiEnv *jvmti) {
   jvmtiError err;

   jvmtiEventCallbacks callbacks;
   memset(&callbacks, 0, sizeof(callbacks));

   callbacks.ResourceExhausted = &resourceExhausted;

   err = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
   if (err != JVMTI_ERROR_NONE) {
      fprintf(stderr, "ERROR: SetEventCallbacks failed: %d\n", err);
      return JNI_ERR;
   }

   err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_RESOURCE_EXHAUSTED, NULL);
   if (err != JVMTI_ERROR_NONE) {
      fprintf(stderr, "ERROR: SetEventNotificationMode failed: %d\n", err);
      return JNI_ERR;
   }
   
   return JNI_OK;
}

int getCount_Threshold() {
   return configuration.count_threshold;
}

int getTime_Threshold() {
   return configuration.time_threshold;
}

void setParameters(char *options) {
   char *subopts;
   char *value;

   //sets defaults
   configuration.count_threshold = 0;
   configuration.time_threshold = 1;

   if (NULL == options)
       return;

   subopts = options;
   while (*subopts != '\0')
      switch (getsubopt (&subopts, tokens, &value)) {
         case COUNT_OPT:
            if (value == NULL)
               abort ();
            configuration.count_threshold = atoi (value);
            break;
         case TIME_OPT:
            if (value == NULL)
               abort ();
            configuration.time_threshold = atoi (value);
            break;
         default:
            /* Unknown suboption. */
            fprintf (stderr, "Unknown suboption '%s'\n", value);
            break;
      }
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
   jvmtiEnv *jvmti;
   
   configuration.signal = SIGKILL;

   jint rc = vm->GetEnv((void **) &jvmti, JVMTI_VERSION);
   if (rc != JNI_OK) {
      fprintf(stderr, "ERROR: GetEnv failed: %d\n", rc);
      return JNI_ERR;
   }
   setParameters(options);	
   return setCallbacks(jvmti);
}


