FROM java:8-jdk

RUN apt-get update && apt-get install -y \
    build-essential \
    python-pip \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN pip install awscli --ignore-installed six
