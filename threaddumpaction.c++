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

#include <limits.h>
#include <sys/types.h>
#include <signal.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "threaddumpaction.h"

void ThreadDumpAction::printThreadDump(JNIEnv* jniEnv, std::ostream *outputStream) {
  jvmtiStackInfo *stackInfo;
  jint threadCount;
  jvmti->GetAllStackTraces(200, &stackInfo, &threadCount);
  for (int i=0;i<threadCount;i++) {
    printThread(&stackInfo[i], outputStream);
  }
}

void ThreadDumpAction::printThread(jvmtiStackInfo *stackInfo, std::ostream *outputStream) {
  jvmtiThreadInfo threadInfo;
  std::string threadName;
  jvmti->GetThreadInfo(stackInfo->thread, &threadInfo);
  threadName.assing(threadInfo.name);
  outputStream <<  " - " << getThreadState(stackInfo);
  printFrames(stackInfo->frame_buffer, stackInfo->frame_count, outputStream);
}

void ThreadDumpAction::printFrames(jvmtiFrameInfo *frames, int frameCount, std::ostream *outputStream) {
  for (int i = 0; i < frameCount; i++) {
  }
}

const std::string ThreadDumpAction::getThreadState(jvmtiStackInfo *stackInfo) {
  switch(stackInfo->state) {
    case JVMTI_THREAD_STATE_ALIVE:
      return "Running";
    case JVMTI_THREAD_STATE_TERMINATED:
      return "Terminated";
    case JVMTI_THREAD_STATE_RUNNABLE:
    return "Runnable";
    case JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER:
    return "Blocked";
    case JVMTI_THREAD_STATE_WAITING:
    case JVMTI_THREAD_STATE_WAITING_INDEFINITELY:
    case JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT:
      return "Waiting";
    case JVMTI_THREAD_STATE_PARKED:
      return "Parked";
    case JVMTI_THREAD_STATE_SUSPENDED:
      return "Suspended";
    case JVMTI_THREAD_STATE_INTERRUPTED:
      return "Interrupted";
    case JVMTI_THREAD_STATE_IN_NATIVE:
      return "Native";
    case JVMTI_THREAD_STATE_VENDOR_1:
    case JVMTI_THREAD_STATE_VENDOR_2:
    case JVMTI_THREAD_STATE_VENDOR_3:
      return "Reserved";
  }
  return "Unknown";
}

ThreadDumpAction::ThreadDumpAction(jvmtiEnv *jvm) {
	jvmti = jvm;
}

void ThreadDumpAction::act(JNIEnv* jniEnv) {
	printThreadDump(jniEnv, &(std::cout));
}
