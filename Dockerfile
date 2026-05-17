FROM gcc:13

RUN apt-get update && apt-get install -y cmake git

WORKDIR /app

RUN git clone https://github.com/CharlieFusion/CPPLab4.git

WORKDIR /app/CPPLab4
RUN mkdir build
WORKDIR /app/CPPLab4/build

RUN cmake .. && make