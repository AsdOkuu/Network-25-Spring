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
}

function create_nodes()
{
	echo 'create_nodes()'
	##########################################
	# your code here 
	##########################################
}


function run_nodes()
{
	echo 'run_node()'
	##########################################
	# your code here, check destroy_nodes() about how to use loop in the script
	##########################################
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
	done
	
	total=${#links[*]}
	echo there are $total items of \<node1 nic1 ip1 node2 nic2 ip2\>
	for ((i=0; i<$total; i=i+6)) do
		echo create the link for ${links[$i]} ${links[$i+1]} ${links[$i+2]} ${links[$i+3]} ${links[$i+4]} ${links[$i+5]}
	##########################################
	# your code here
	##########################################
	done
}


function stop_nodes()
{
	echo 'stop_nodes()'
	##########################################
	# your code here
	##########################################

}


function destroy_nodes()
{
	echo 'destroy_nodes()'
	for h in ${nodes[@]}; do
		echo destroy $h
	##########################################
	# your code here
	##########################################
	done
}
function destroy_image()
{
	echo 'destroy_image()'
	echo destroy image node
	##########################################
	# your code here
	##########################################
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
	done
	# add routing rules
	##########################################
	# your code here
	##########################################
	# h1 -> h2 
	# h2 -> h1
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

