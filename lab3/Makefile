setup:
	./lab.sh -ci 
	./lab.sh -cn 
	./lab.sh -rn 
	./lab.sh -cl 
	./lab.sh -cr 

destroy:
	./lab.sh -sn
	./lab.sh -dn
	./lab.sh -di 

test:
	docker exec -it h1 ping 111.0.6.2 -c 5
	docker exec -it h1 traceroute 111.0.6.2
	docker exec -it h2 traceroute 111.0.0.1
