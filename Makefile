ifndef JAVA_HOME
    $(error JAVA_HOME not set)
endif

ifeq ($(shell uname -s),Darwin)
  INCLUDE= -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/darwin"
else
  INCLUDE= -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux"
endif

CFLAGS=-Wall -Werror -fPIC -shared $(INCLUDE)
TARGET=libjvmkill.so

.PHONY: all clean test

all:
	gcc $(CFLAGS) -o $(TARGET) jvmkill.c
	chmod 644 $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.class
	rm -f tests

test: all
	gcc -g -Wall -Werror $(INCLUDE) -ldl -o tests tests.c
	./tests 
	$(JAVA_HOME)/bin/javac JvmKillTestThreads.java
	$(JAVA_HOME)/bin/java -Xmx1m \
	    -agentpath:$(PWD)/$(TARGET) \
	    -cp $(PWD) JvmKillTestThreads
	$(JAVA_HOME)/bin/java -Xmx1m \
	    -agentpath:$(PWD)/$(TARGET)=time=10,count=2 \
	    -cp $(PWD) JvmKillTestThreads
	$(JAVA_HOME)/bin/javac JvmKillTest.java
	$(JAVA_HOME)/bin/java -Xmx1m \
	    -agentpath:$(PWD)/$(TARGET)=time=10,count=2 \
	    -cp $(PWD) JvmKillTest

