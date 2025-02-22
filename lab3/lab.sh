#!/bin/bash

source topo.conf

function create_image()
{
	echo 'create_image()'
	echo create image node
	##########################################
	# your code here 
	##########################################
	docker image build -t node .
    docker images
}

function create_nodes()
{
	echo 'create_nodes()'
	##########################################
	# your code here 
	##########################################
    # Loop to create
    for h in ${nodes[@]}; do
        docker container create --cap-add NET_ADMIN --name $h node
    done
    # List all
    docker container list -a
}


function run_nodes()
{
	echo 'run_node()'
	##########################################
	# your code here, check destroy_nodes() about how to use loop in the script
	##########################################
    for h in ${nodes[@]}; do
        docker container start $h
    done
}

function create_links()
{
	echo 'create_links()'
	echo "expose each node's namespace"
	
	total=${#nodes[*]}
	echo there are $total nodes
	for ((i=0; i<$total; i=i+1)) do
		echo expose "${nodes[$i]}'s" namespace
	##########################################
	# your code here
	##########################################
        ns=`docker inspect -f '{{.State.Pid}}' ${nodes[$i]}`
        # Expose netns
        ln -s /proc/$ns/ns/net /var/run/netns/$ns
	done
	
	total=${#links[*]}
	echo there are $total items of \<node1 nic1 ip1 node2 nic2 ip2\>
	for ((i=0; i<$total; i=i+6)) do
		echo create the link for ${links[$i]} ${links[$i+1]} ${links[$i+2]} ${links[$i+3]} ${links[$i+4]} ${links[$i+5]}
	##########################################
	# your code here
	##########################################
        # Create linked veth
        ip link add ${links[$i+1]} type veth peer name ${links[$i+4]}
        # Configure node1
        ns=`docker inspect -f '{{.State.Pid}}' ${links[$i]}`
        ip link set ${links[$i+1]} netns $ns
        ip netns exec $ns ip link set ${links[$i+1]} up
        ip netns exec $ns ip addr add ${links[$i+2]} dev ${links[$i+1]}
        # Configure node2
        ns=`docker inspect -f '{{.State.Pid}}' ${links[$i+3]}`
        ip link set ${links[$i+4]} netns $ns
        ip netns exec $ns ip link set ${links[$i+4]} up
        ip netns exec $ns ip addr add ${links[$i+5]} dev ${links[$i+4]}
	done
}


function stop_nodes()
{
	echo 'stop_nodes()'
	##########################################
	# your code here
	##########################################
    for h in ${nodes[@]}; do
        docker container stop $h
    done
}


function destroy_nodes()
{
	echo 'destroy_nodes()'
	for h in ${nodes[@]}; do
		echo destroy $h
	##########################################
	# your code here
	##########################################
        docker container rm $h
	done
}
function destroy_image()
{
	echo 'destroy_image()'
	echo destroy image node
	##########################################
	# your code here
	##########################################
    docker image rm node
}

function configure_routes()
{
	echo 'configure_route()'
	# configure routers with ip_forward 1
	for h in ${nodes[@]}; do
		echo configure $h with ip_forward
	##########################################
	# your code here
	##########################################
        docker exec -it $h bash -c "echo 1 > /proc/sys/net/ipv4/ip_forward"
        docker exec -it $h cat /proc/sys/net/ipv4/ip_forward
	done
	# add routing rules
	##########################################
	# your code here
	##########################################
	# h1 -> h2 
    docker exec -it h1 route add -net 111.0.6.0 netmask 255.255.255.0 gateway 111.0.0.2
    docker exec -it r1 route add -net 111.0.6.0 netmask 255.255.255.0 gateway 111.0.1.2
    docker exec -it r2 route add -net 111.0.6.0 netmask 255.255.255.0 gateway 111.0.2.2
    docker exec -it r3 route add -net 111.0.6.0 netmask 255.255.255.0 gateway 111.0.3.2
	# h2 -> h1
    docker exec -it h2 route add -net 111.0.0.0 netmask 255.255.255.0 gateway 111.0.6.1
    docker exec -it r5 route add -net 111.0.0.0 netmask 255.255.255.0 gateway 111.0.5.1
    docker exec -it r4 route add -net 111.0.0.0 netmask 255.255.255.0 gateway 111.0.4.1
}

case $1 in 
	"-ci")
		create_image
		;;
	"-cn")
		create_nodes
		;;
	"-rn")
		run_nodes
		;;
	"-cl")
		create_links
		;;
	"-sn")
		stop_nodes
		;;
	"-dn")
		destroy_nodes
		;;
	"-di")
		destroy_image
		;;
	"-cr")
		configure_routes
		;;
	*)
		echo "input error !"
		;;
esac

