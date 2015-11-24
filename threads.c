#include "threads.h"

static char* getFileName(jvmtiEnv* jvmti, jclass declaringClass) {
  char *fileName;

  (*jvmti)->GetSourceFileName(jvmti, declaringClass, &fileName);
  return fileName;
}

static char* clearClassName(char* className) {
  int i;
  char* cleanClassName = strdup(className + 1);

  i = 0;
  while (cleanClassName[i]) {
    if (cleanClassName[i] == '/') {
      cleanClassName[i] = '.';
    } else if (cleanClassName[i] == ';') {
      cleanClassName[i] = '.';
    }
    i++;
  }
  return cleanClassName;
}

static void JNICALL printFrame(jvmtiEnv* jvmti, jvmtiFrameInfo frame) {
  char *methodName, *className, *cleanClassName, *fileName;
  int lineNumber;
  jclass declaringClass;



  (*jvmti)->GetMethodName(jvmti, frame.method, &methodName, NULL, NULL);
  (*jvmti)->GetMethodDeclaringClass(jvmti, frame.method, &declaringClass);
  (*jvmti)->GetClassSignature(jvmti, declaringClass, &className, NULL);
  lineNumber = 1;
  fileName = getFileName(jvmti, declaringClass);

  cleanClassName = clearClassName(className);

  if (lineNumber) {
    fprintf(stdout, "\tat %s%s(%s:%d)\n", cleanClassName, methodName, fileName, lineNumber);
  } else {
    fprintf(stdout, "\tat %s%s(%s)\n", cleanClassName, methodName, fileName);
  }
  (*jvmti)->Deallocate(jvmti, (unsigned char*) methodName);
  (*jvmti)->Deallocate(jvmti, (unsigned char*) className);
  (*jvmti)->Deallocate(jvmti, (unsigned char*) fileName);
  free(cleanClassName);
}

static char* getThreadState(jint state) {
    if (state & JVMTI_THREAD_STATE_SUSPENDED) {
      return "SUSPENDED";
    } else if (state & JVMTI_THREAD_STATE_INTERRUPTED) {
      return "INTERRUPTED";
    } else if (state & JVMTI_THREAD_STATE_IN_NATIVE) {
      return "NATIVE";
    } else if (state & JVMTI_THREAD_STATE_RUNNABLE) {
      return "RUNNABLE";
    } else if (state & JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER) {
      return "BLOCKED";
    } else if (state & JVMTI_THREAD_STATE_IN_OBJECT_WAIT) {
      return "WAITING";
    } else if (state & JVMTI_THREAD_STATE_PARKED) {
      return "PARKED";
    } else if (state & JVMTI_THREAD_STATE_SLEEPING) {
      return "SLEEPING";
    } else {
      return "UNKNOWN";
    }
}

void printThreadDump(jvmtiEnv *jvmti) {
  jvmtiStackInfo *stack_info;
  jint thread_count;
  int ti;
  //jvmtiError err;
  jvmtiThreadInfo threadInfo;

  jvmtiCapabilities caps;
  memset(&caps, '\0', sizeof(caps));
  caps.can_get_source_file_name = 1;
  caps.can_get_line_numbers = 1;
  (*jvmti)->AddCapabilities(jvmti, &caps);

  fprintf(stdout, "\n");
  (*jvmti)->GetAllStackTraces(jvmti, 150, &stack_info, &thread_count);
  fprintf(stdout, "Dumping thread state for %d threads\n\n", thread_count);
  for (ti = 0; ti < thread_count; ++ti) {
    jvmtiStackInfo *infop = &stack_info[ti];
    jthread thread = infop->thread;
    jint state = infop->state;
    jvmtiFrameInfo *frames = infop->frame_buffer;
    int fi;
    const char *threadState = getThreadState(state);

    (*jvmti)->GetThreadInfo(jvmti, thread, &threadInfo);
    fprintf(stdout, "#%d - %s - %s", ti + 1, threadInfo.name, threadState);
    fprintf(stdout, "\n");
    (*jvmti)->Deallocate(jvmti, (void*) threadInfo.name);

    for (fi = 0; fi < infop->frame_count; fi++) {
      printFrame(jvmti, frames[fi]);
    }
    fprintf(stdout,"\n");
  }
  fprintf(stdout,"\n\n");
  /* this one Deallocate call frees all data allocated by GetAllStackTraces */
  (*jvmti)->Deallocate(jvmti, (void*) stack_info);
}
