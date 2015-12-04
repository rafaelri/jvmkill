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
#include <string.h>
#include <unistd.h>

#include <jvmti.h>

/**
 * Configuration struct that holds agent configuration 
 */
struct Configuration {
   int count_threshold; //configured event count limit
   int time_threshold; //configured time limit for keeping events
   int signal; //configured unix signal to be fired
};
#ifdef __cplusplus
extern "C" {
#endif
void setSignal(int signal); //function for overriding unix signal sent when the threshold is reached

void resourceExhausted(
      jvmtiEnv *jvmti_env,
      JNIEnv *jni_env,
      jint flags,
      const void *reserved,
      const char *description); //resourceExhausted callback registered in the JVM

int getTime_Threshold(); //provides read access to agent Time threshold configuration
int getCount_Threshold(); //provides read access to agent Time threshold configuration
void setParameters(char *options); //function for configuring agent parameters 
#ifdef __cplusplus
}
#endif

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved); //Agent load callback called by the JVM
