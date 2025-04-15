#!/bin/bash

# the network topology is a chain: host1 <-> device1 <-> device2 <-> device3 <-> device4 <-> host2
nodes=(host1 device1 device2 device3 device4 host2)

links=( host1 host1-eth0 66:77:88:00:01:01 device1 device1-eth0 66:77:88:00:01:02 \
    device1 device1-eth1 66:77:88:00:02:01 device2 device2-eth0 66:77:88:00:02:02 \
    device2 device2-eth1 66:77:88:00:03:01 device3 device3-eth0 66:77:88:00:03:02 \
    device3 device3-eth1 66:77:88:00:04:01 device4 device4-eth0 66:77:88:00:04:02 \
    device4 device4-eth1 66:77:88:00:05:01 host2 host2-eth0 66:77:88:00:05:02 )

# IP address configuration: interface ip_address
ips=(host1-eth0 10.0.1.1/24 \
    device1-eth0 10.0.1.2/24 \
    device1-eth1 10.0.2.1/24 \
    device2-eth0 10.0.2.2/24 \
    device2-eth1 10.0.3.1/24 \
    device3-eth0 10.0.3.2/24 \
    device3-eth1 10.0.4.1/24 \
    device4-eth0 10.0.4.2/24 \
    device4-eth1 10.0.5.1/24 \
    host2-eth0 10.0.5.2/24 \
)

arp=(host1 66:77:88:00:01:01 10.0.1.1 \
    host2 66:77:88:00:05:02 10.0.5.2 )

host_route=(host1 10.0.0.0/16 host1-eth0 \
    host2 10.0.0.0/16 host2-eth0 )

rules=(device1 10.0.1.0/24 device1-eth0 0 \
    device4 10.0.5.0/24 device4-eth1 0 )

setup() {
    # create and start the nodes
    for node in ${nodes[@]}; do
        echo "Creating and starting $node"
        docker container create --cap-add NET_ADMIN --name $node node
        docker container start $node
    done

    # create veth links between containers
    for ((i=0; i<${#links[@]}; i+=6)); do
        container1=${links[i]}
        veth1=${links[i+1]}
        mac1=${links[i+2]}
        container2=${links[i+3]}
        veth2=${links[i+4]}
        mac2=${links[i+5]}

        echo "Creating link between $container1($veth1) and $container2($veth2)"
        
        # Create veth pair
        sudo ip link add $veth1 type veth peer name $veth2
        
        # Set MAC addresses
        sudo ip link set $veth1 address $mac1
        sudo ip link set $veth2 address $mac2
        
        # Move veth interfaces to containers
        sudo ip link set $veth1 netns $(docker inspect -f '{{.State.Pid}}' $container1)
        sudo ip link set $veth2 netns $(docker inspect -f '{{.State.Pid}}' $container2)
        
        # Bring interfaces up
        sudo docker exec $container1 ip link set $veth1 up
        sudo docker exec $container2 ip link set $veth2 up
    done

    # configure IP addresses
    for ((i=0; i<${#ips[@]}; i+=2)); do
        interface=${ips[i]}
        ip_addr=${ips[i+1]}
        container=${interface%-*}  # extract container name from interface
        
        echo "Configuring $interface with $ip_addr in $container"
        sudo docker exec $container ip addr add $ip_addr dev $interface
    done

    # force host to route 10.0/16 to eth0
    for ((i=0; i<${#host_route[@]}; i+=3)); do
        container=${host_route[i]}
        target_net=${host_route[i+1]}
        interface=${host_route[i+2]}        
        echo "Configuring route in $container for $target_net via $interface"
        sudo docker exec $container ip route add $target_net dev $interface
    done

    # configure ARP mappings in host containers
    for node in ${nodes[@]}; do
        if [[ $node == host* ]]; then
            echo "Configuring ARP in $node"
            for ((i=0; i<${#arp[@]}; i+=3)); do
                container=${arp[i]}
                mac=${arp[i+1]}
                ip_addr=${arp[i+2]%/*}  # remove /24 suffix
                if [[ $container != $node ]]; then
                    sudo docker exec $node arp -s $ip_addr $mac
                fi
            done
        fi
    done
    
    # generate router rules files
    declare -A container_rules
    for ((i=0; i<${#rules[@]}; i+=4)); do
        container=${rules[i]}
        target_net=${rules[i+1]}
        interface=${rules[i+2]}
        distance=${rules[i+3]}
        container_rules[$container]+="$target_net $interface $distance\n"
    done
    
    for container in "${!container_rules[@]}"; do
        echo "Generating rules for $container"
        tmp_file=$(mktemp)
        echo -e "${container_rules[$container]}" > $tmp_file
        docker cp $tmp_file $container:/rules.txt
        rm $tmp_file
    done
}

clean() {
    # remove all containers
    for node in ${nodes[@]}; do
        echo remove $node
        docker container rm -f $node
    done
}

# handle command line arguments
case "$1" in
    "setup")
        setup
        ;;
    "clean")
        clean
        ;;
    *)
        echo "Usage: $0 {setup|clean}"
        exit 1
        ;;
esac