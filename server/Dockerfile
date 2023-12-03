FROM ubuntu:latest
RUN apt-get update;
RUN apt-get install -y python3; apt-get install -y pip;
RUN pip install flask;
WORKDIR /home
CMD ["flask","run"]