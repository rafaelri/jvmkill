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
#include <stdio.h>
#include <stdlib.h>

#include <jvmti.h>

#include "agentcontroller.h"
#include "parameters.h"
#include "heuristic.h"
#include "threshold.h"
#include "heaphistogramaction.h"
#include "action.h"
static int MockPrintHeapActionCount = 0;
static int MockPrintHeapActionRunCount = 0;
AgentController* agentController;
jvmtiEnv* jvm;
//mock
HeapHistogramAction::HeapHistogramAction(jvmtiEnv* jvm) {
	MockPrintHeapActionCount++;
}
void HeapHistogramAction::act() {
	MockPrintHeapActionRunCount++;
}

//incluir mock com o createNewHeapHistogramAction para testar (nao existe instanceof no C++)
void setup() {
	agentController = new AgentController(NULL);
}

bool testDoesNotAddHeapActionWhenOff() {
	AgentParameters params;
	params.print_heap_histogram = false;
	agentController->setParameters(params);
  bool passed = MockPrintHeapActionCount > 0;
    if (!passed) {
       fprintf(stdout, "testTriggersIfThresholdExceeded FAILED\n");
    }
	return passed;
}



int main() {
  setup();
	bool result = (testDoesNotAddHeapActionWhenOff());
	if (result) {
       fprintf(stdout, "SUCCESS\n");
	   exit(EXIT_SUCCESS);
	}
	else {
    	fprintf(stdout, "FAILURE\n");
    	exit(EXIT_FAILURE);
	}
}
