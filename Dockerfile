FROM gcc:9


RUN apt-get update && \
    apt-get install -y libssl-dev \
                       libboost-system-dev \
                       libboost-thread-dev

COPY . /root/projects/helloworld
WORKDIR /root/projects/helloworld

RUN mkdir -p build/ && cd build/ && rm -rf * 
RUN cd build/ && gcc -I.. -o HttpLinux ../ServerLogic.cpp ../ServerMain.cpp -lstdc++ -lboost_system -lboost_thread -lpthread

#change this to whatever port you choose to run your server on
EXPOSE 8080 