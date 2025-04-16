# Base images 基础镜像
FROM ubuntu:22.04

# RUN timedatectl set-timezone Asia/Shanghai'
COPY sources.list /etc/apt/sources.list
RUN apt update 

RUN echo -e "6\n70\n" | apt install tzdata 
RUN apt-get install sudo -y && sudo apt-get install vim \
    iproute2 traceroute autoconf make gcc libpcap-dev \
    openssh-server openssh-client ssh  iputils-ping net-tools \
    tcpdump psmisc  iperf -y
RUN apt-get install bridge-utils -y


CMD ["tail", "-f", "/dev/null"]


