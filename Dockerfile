FROM ubuntu:latest
RUN apt-get update;
RUN apt-get install -y cmake; apt-get install -y gcc-arm-none-eabi
RUN apt-get install -y newlib-arm-none-eabi; apt-get install -y libstdc++-arm-none-eabi-newlib
RUN apt-get install -y python3; apt-get install -y build-essential
WORKDIR /home
CMD ["/bin/sh"]