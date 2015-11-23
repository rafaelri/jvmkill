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
#include <time.h>

#include "jvmkill.h"
#include "threads.c"

enum {
    TIME_OPT = 0,
    COUNT_OPT = 1
};
 
char *tokens[] = {
    "time",
    "count"
};

static int count;
static time_t current_timestamp;
static struct Configuration configuration;

void setSignal(int signal) {
   configuration.signal = signal;
}
void
resourceExhausted(
      jvmtiEnv *jvmti_env,
      JNIEnv *jni_env,
      jint flags,
      const void *reserved,
      const char *description) {
   time_t evaluated = time(NULL)/configuration.time_threshold;
   fprintf(stderr, "ResourceExhausted!\n");
   if (current_timestamp == evaluated) {
     count++;
   }
   else {
     count=1;
     current_timestamp = evaluated;
   }
   if (count > configuration.count_threshold) {
	printThreadDump(jvmti_env);
   	kill(getpid(), configuration.signal);
        fprintf(stderr, "killing current process\n");
   }
}

int setCallbacks(jvmtiEnv *jvmti) {
   jvmtiError err;

   jvmtiEventCallbacks callbacks;
   memset(&callbacks, 0, sizeof(callbacks));

   callbacks.ResourceExhausted = &resourceExhausted;

   err = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
   if (err != JVMTI_ERROR_NONE) {
      fprintf(stderr, "ERROR: SetEventCallbacks failed: %d\n", err);
      return JNI_ERR;
   }

   err = (*jvmti)->SetEventNotificationMode(
         jvmti, JVMTI_ENABLE, JVMTI_EVENT_RESOURCE_EXHAUSTED, NULL);
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
            fprintf (stderr, "Unknown suboption `%s'\n", value);
            break;
      }
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
   jvmtiEnv *jvmti;

   configuration.signal = SIGKILL;

   jint rc = (*vm)->GetEnv(vm, (void **) &jvmti, JVMTI_VERSION);
   if (rc != JNI_OK) {
      fprintf(stderr, "ERROR: GetEnv failed: %d\n", rc);
      return JNI_ERR;
   }
   setParameters(options);	
   return setCallbacks(jvmti);
}


