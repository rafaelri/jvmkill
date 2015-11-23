#include "threads.h"

static void JNICALL printFrame(jvmtiEnv* jvmti, jvmtiFrameInfo frame) {
  jvmtiError err;
  char *methodName, *className, *cleanClassName;
  char *fileName;
  int si, li, lineNumber;
  jclass declaringClass;
  jint locationCount;
  jvmtiLineNumberEntry* locationTable;



  (*jvmti)->GetMethodName(jvmti, frame.method, &methodName, NULL, NULL);
  (*jvmti)->GetMethodDeclaringClass(jvmti, frame.method, &declaringClass);
  (*jvmti)->GetClassSignature(jvmti, declaringClass, &className, NULL);
  err = (*jvmti)->GetSourceFileName(jvmti, declaringClass, &fileName);
  if (err == JVMTI_ERROR_NATIVE_METHOD || err == JVMTI_ERROR_ABSENT_INFORMATION) {
    fileName = strdup("Unknown");

  } else {
    char *temp;

    temp = strdup(fileName);
    //(*jvmti)->Deallocate(jvmti, (void*) fileName);
    fileName = temp;
  }
  err = (*jvmti)->GetLineNumberTable(jvmti, frame.method, &locationCount, &locationTable);
  if (err == JVMTI_ERROR_NATIVE_METHOD || err == JVMTI_ERROR_ABSENT_INFORMATION) {
    lineNumber = 0;
  } else {
    lineNumber = 0;
    for (li = 0; li < locationCount; li++) {
      if (locationTable[li].start_location > frame.location) {
        break;
      }
      lineNumber = locationTable[li].line_number;
    }
    //(*jvmti)->Deallocate(jvmti, (void*) locationTable);
  }

  cleanClassName = strdup(className + 1);
  si = 0;
  while (cleanClassName[si]) {
    if (cleanClassName[si] == '/') {
      cleanClassName[si] = '.';
    } else if (cleanClassName[si] == ';') {
      cleanClassName[si] = '.';
    }
    si++;
  }

  if (lineNumber) {
    fprintf(stdout, "\tat %s%s(%s:%d)\n", cleanClassName, methodName, fileName, lineNumber);
  } else {
    fprintf(stdout, "\tat %s%s(%s)\n", cleanClassName, methodName, fileName);
  }
  (*jvmti)->Deallocate(jvmti, (void*) methodName);
  (*jvmti)->Deallocate(jvmti, (void*) className);
  free(fileName);
  free(cleanClassName);
}


void printThreadDump(jvmtiEnv *jvmti) {
  jvmtiStackInfo *stack_info;
  jint thread_count;
  int ti;
  //jvmtiError err;
  jvmtiThreadInfo threadInfo;

  fprintf(stdout, "\n");
  (*jvmti)->GetAllStackTraces(jvmti, 150, &stack_info, &thread_count);
  fprintf(stdout, "Dumping thread state for %d threads\n\n", thread_count);
  for (ti = 0; ti < thread_count; ++ti) {
    jvmtiStackInfo *infop = &stack_info[ti];
    jthread thread = infop->thread;
    jint state = infop->state;
    jvmtiFrameInfo *frames = infop->frame_buffer;
    int fi;
    const char *threadState;

    if (state & JVMTI_THREAD_STATE_SUSPENDED) {
      threadState = "SUSPENDED";
    } else if (state & JVMTI_THREAD_STATE_INTERRUPTED) {
      threadState = "INTERRUPTED";
    } else if (state & JVMTI_THREAD_STATE_IN_NATIVE) {
      threadState = "NATIVE";
    } else if (state & JVMTI_THREAD_STATE_RUNNABLE) {
      threadState = "RUNNABLE";
    } else if (state & JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER) {
      threadState = "BLOCKED";
    } else if (state & JVMTI_THREAD_STATE_IN_OBJECT_WAIT) {
      threadState = "WAITING";
    } else if (state & JVMTI_THREAD_STATE_PARKED) {
      threadState = "PARKED";
    } else if (state & JVMTI_THREAD_STATE_SLEEPING) {
      threadState = "SLEEPING";
    } else {
      threadState = "UNKNOWN";
    }

    (*jvmti)->GetThreadInfo(jvmti, thread, &threadInfo);
    fprintf(stdout, "#%d - %s - %s", ti + 1, threadInfo.name, threadState);
    fprintf(stdout, "\n");
    //(*jvmti)->Deallocate(jvmti, (void*) threadInfo);

    for (fi = 0; fi < infop->frame_count; fi++) {
      printFrame(jvmti, frames[fi]);
    }
    fprintf(stdout,"\n");
  }
  fprintf(stdout,"\n\n");
  /* this one Deallocate call frees all data allocated by GetAllStackTraces */
  (*jvmti)->Deallocate(jvmti, (void*) stack_info);
}
