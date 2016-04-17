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
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parameters.h"
#include "parametersparser.h"


ParametersParser *parser;

void setup() {
    parser = new ParametersParser();
}

void teardown() {
}


bool testsDefaults() {
	AgentParameters params = parser->parse(strdup(""));
	return ((params.time_threshold == 1) && (params.count_threshold == 0));
}

bool testsParsesTimeThreshold() {
	AgentParameters params = parser->parse(strdup("time=99"));
	return (params.time_threshold == 99);
}

bool testsParsesCountThreshold() {
	AgentParameters params = parser->parse(strdup("count=88"));
	return (params.time_threshold == 88);
}

bool testsParsesPrintHeapHistogramOn() {
	AgentParameters params = parser->parse(strdup("printHeapHistogram=1"));
	return (params.print_heap_histogram == true);
}

bool testsParsesPrintHeapHistogramOff() {
	AgentParameters params = parser->parse(strdup("printHeapHistogram=0"));
	return (params.print_heap_histogram == false);
}

int main() {
	setup();
	bool result = testsDefaults() && testsParsesTimeThreshold() && testsParsesPrintHeapHistogramOn() && testsParsesPrintHeapHistogramOff();
	teardown();
	if (result) {
       fprintf(stdout, "SUCCESS\n");
	   exit(EXIT_SUCCESS);
	}
	else {
       fprintf(stdout, "FAILURE\n");
       exit(EXIT_FAILURE);
	}
}
