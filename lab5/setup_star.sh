#!/bin/bash

# the network topology in the homework
nodes=(device host1 host2 host3 host4)

links=( device device-eth0 66:77:88:00:00:01 host1 host1-eth0 66:77:88:00:00:02 \
    device device-eth1 66:77:88:00:00:03 host2 host2-eth0 66:77:88:00:00:04 \
    device device-eth2 66:77:88:00:00:05 host3 host3-eth0 66:77:88:00:00:06 \
    device device-eth3 66:77:88:00:00:07 host4 host4-eth0 66:77:88:00:00:08 )

# IP address configuration: interface ip_address

ips=(host1-eth0 10.0.0.1/24 \
    host2-eth0 10.0.0.2/24 \
    host3-eth0 10.0.0.3/24 \
    host4-eth0 10.0.0.4/24 \
)

arp=(host1 66:77:88:00:00:02 10.0.0.1 \
    host2 66:77:88:00:00:04 10.0.0.2 \
    host3 66:77:88:00:00:06 10.0.0.3 \
    host4 66:77:88:00:00:08 10.0.0.4 )

connections=(host1 10.0.0.1 10001 host4 10.0.0.4 40001 file1.txt 500000\
    host2 10.0.0.2 20001 host4 10.0.0.4 40002 file2.txt 500000 )

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
        # echo ip link add $veth1 type veth peer name $veth2
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

    # Create bridge and add all device interfaces
    echo "Creating bridge in device container"
    sudo docker exec device brctl addbr br0
    for i in {0..3}; do
        sudo docker exec device brctl addif br0 device-eth$i
        sudo docker exec device ip link set device-eth$i up
    done
    sudo docker exec device ip link set br0 up

    # Generate connection config files
    for ((i=0; i<${#connections[@]}; i+=8)); do
        container1=${connections[i]}
        ip1=${connections[i+1]}
        port1=${connections[i+2]}
        container2=${connections[i+3]}
        ip2=${connections[i+4]}
        port2=${connections[i+5]}
        filename=${connections[i+6]}
        line_count=${connections[i+7]}

        # Generate random number file
        temp_file=$(mktemp)
        shuf -i 1-1000000 -n $line_count > $temp_file
        
        # Copy file to container1
        docker cp $temp_file ${container1}:/${filename}
        file_size=$(wc -m < $temp_file)
        rm $temp_file

        # Create connections.txt in container1
        docker exec $container1 sh -c "echo \"$ip1 $port1 $ip2 $port2 $filename $file_size\" >> /connections.txt"

        # Create connections.txt in container2
        docker exec $container2 sh -c "echo \"$ip2 $port2 $ip1 $port1 $filename $file_size\" >> /connections.txt"
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




