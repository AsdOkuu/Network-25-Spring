#!/bin/bash
apt install libpcap-dev

systemctl daemon-reload
systemctl stop docker.service
systemctl stop docker.socket
systemctl start docker.service
systemctl start docker.socket

docker image build -t node .

docker container create --cap-add NET_ADMIN --name n1 node
docker container create --cap-add NET_ADMIN --name n2 node
docker start n1
docker start n2

ip link add n1-eth0 type veth peer name n2-eth0

ns1=`docker inspect -f '{{.State.Pid}}' n1`
ln -s /proc/$ns1/ns/net /var/run/netns/$ns1
ip link set n1-eth0 netns $ns1
ip netns exec $ns1 ip link set n1-eth0 up
ip netns exec $ns1 ip addr add 10.0.0.1/24 dev n1-eth0

ns2=`docker inspect -f '{{.State.Pid}}' n2`
ln -s /proc/$ns2/ns/net /var/run/netns/$ns2
ip link set n2-eth0 netns $ns2
ip netns exec $ns2 ip link set n2-eth0 up
ip netns exec $ns2 ip addr add 10.0.0.2/24 dev n2-eth0

