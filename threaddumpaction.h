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

#ifndef threaddumpaction_h
#define threaddumpaction_h

#include "action.h"
#include <jni.h>
#include <jvmti.h>
using namespace std;

class ThreadDumpAction: public Action
{
public:
   ThreadDumpAction(jvmtiEnv *jvmti);

   void act(JNIEnv* jniEnv);

private:
   jvmtiEnv* jvmti;

   void printThreadDump(JNIEnv* jniEnv, std::ostream *outputStream);
   void printThread(jvmtiStackInfo *stackInfo, std::ostream *outputStream);
   void printFrames(jvmtiFrameInfo *frames, int frameCount, std::ostream *outputStream);
   const std::string getThreadState(jvmtiStackInfo *stackInfo);
};

#endif // threaddumpaction_h
