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
  jvmtiThreadGroupInfo threadGroupInfo;
  jvmti->GetThreadInfo(stackInfo->thread, &threadInfo);
  jvmti->GetThreadGroupInfo(threadInfo.thread_group, &threadGroupInfo);
  *outputStream << threadInfo.name << " [" << threadGroupInfo.name << "] (" << getThreadState(stackInfo) << ")\n";
  jvmti->Deallocate((unsigned char *)threadInfo.name);
  jvmti->Deallocate((unsigned char *)threadGroupInfo.name);
  printFrames(stackInfo->frame_buffer, stackInfo->frame_count, outputStream);
}

//refactor
void ThreadDumpAction::printFrames(jvmtiFrameInfo *frames, int frameCount, std::ostream *out) {
  jclass declaringClass;
  char *methodName, *className, *fileName;
  jvmtiFrameInfo frame;
  int lineNumber;

  for (int i = 0; i < frameCount; i++) {
    frame=frames[i];
    jvmti->GetMethodDeclaringClass(frame.method, &declaringClass);

    jvmti->GetClassSignature(declaringClass, &className, NULL);
    *out << "\t" << className;
    jvmti->Deallocate((unsigned char *)className);

//checar erros (não imprimir se der erro... ver com o que substituir)
    jvmti->GetMethodName(frame.method, &methodName, NULL, NULL);
    *out << methodName;
    jvmti->Deallocate((unsigned char *)methodName);

    jvmtiError err = jvmti->GetSourceFileName(declaringClass, &fileName);
    if (err != JVMTI_ERROR_NONE) {
        *out << "(Unknown)";
    } else {
        *out << "(" << fileName;
        jvmti->Deallocate((unsigned char *)fileName);
        err = getLineNumber(frame, &lineNumber);
        if (err != JVMTI_ERROR_NONE) {
          if (err == JVMTI_ERROR_NATIVE_METHOD) {
            *out << ":native)";
          }
          else {
            *out << ")";
          }
        }
        else {
          *out << ":" << std::to_string(lineNumber) << ")";
        }
    }
    *out << "\n";
  }
}

jvmtiError ThreadDumpAction::getLineNumber(jvmtiFrameInfo frame, int *lineNumber) {
    jint locationCount;
    jvmtiLineNumberEntry* locationTable;
    jvmtiError err = jvmti->GetLineNumberTable(frame.method, &locationCount, &locationTable);
  if (err == JVMTI_ERROR_NONE) {
      *lineNumber = 0;
      for (int i = 0; i < locationCount; i++) {
        if (locationTable[i].start_location > frame.location) {
          break;
        }
        *lineNumber = locationTable[i].line_number;
      }
      jvmti->Deallocate((unsigned char *)locationTable);
  }
  return err;
}

const std::string ThreadDumpAction::getThreadState(jvmtiStackInfo *stackInfo) {
  int state = stackInfo->state;
  if (state & JVMTI_THREAD_STATE_IN_NATIVE)
    return "Native";
  if (state & JVMTI_THREAD_STATE_INTERRUPTED)
    return "Interrupted";
  if (state & JVMTI_THREAD_STATE_SUSPENDED)
    return "Suspended";
  if (state & JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER)
    return "Blocked";
  if (state & JVMTI_THREAD_STATE_PARKED)
    return "Parked";
  if (state & JVMTI_THREAD_STATE_SLEEPING)
    return "Sleeping"; //sleeping returns also waiting and waiting_with_timeout
  if (state & JVMTI_THREAD_STATE_WAITING)
    return "Waiting";
  if (state & JVMTI_THREAD_STATE_RUNNABLE)
    return "Runnable";
  if (state & JVMTI_THREAD_STATE_TERMINATED)
    return "Terminated";
  if ((state & JVMTI_THREAD_STATE_ALIVE) == 0)
    return "New";
  return "Unknown";
}

ThreadDumpAction::ThreadDumpAction(jvmtiEnv *jvm) {
  jvmtiCapabilities capabilities;

	/* Get/Add JVMTI capabilities */
	int err = jvm->GetCapabilities(&capabilities);
	if (err != JVMTI_ERROR_NONE) {
		fprintf(stderr, "ERROR: GetCapabilities failed: %d\n", err);
		throw new std::runtime_error("GetCapabilities failed");
    }

	capabilities.can_get_line_numbers = 1;
  capabilities.can_get_source_file_name = 1;

	err = jvm->AddCapabilities(&capabilities);
	if (err != JVMTI_ERROR_NONE) {
		fprintf(stderr, "ERROR: AddCapabilities failed: %d\n", err);
		throw new std::runtime_error("AddCapabilities failed");
    }

    jvmti = jvm;
}

void ThreadDumpAction::act(JNIEnv* jniEnv) {
    printThreadDump(jniEnv, &(std::cout));
    std::cout.flush();
}
