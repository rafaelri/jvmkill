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
 * === time_threshold is expressed in seconds ===
 */
struct Configuration {
   int count_threshold;
   int time_threshold; 
   int signal; 
};
#ifdef __cplusplus
extern "C" {
#endif
/*
 * function for overriding unix signal sent when the threshold is reached
 * === Mainly used on unit tests ===
 */
void setSignal(int signal); 

/*
 * resourceExhausted callback registered in the JVM
 */
void resourceExhausted(
      jvmtiEnv *jvmti_env,
      JNIEnv *jni_env,
      jint flags,
      const void *reserved,
      const char *description); 

int getTime_Threshold(); 
int getCount_Threshold(); 

/*
 * function that parses agent parameters
 */
void setParameters(char *options); 
#ifdef __cplusplus
}
#endif

/*
 * Agent load callback called by the JVM
 */
JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved); 
