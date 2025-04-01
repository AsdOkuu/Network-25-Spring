all:
	gcc host.c -o host -lpcap
	docker container cp host n1:.
	docker container cp host n2:.
	docker exec n2 ./host receiver n2-eth0  &
	docker exec n1 ./host sender n1-eth0  

setup:
	./setup.sh

clean:
	./clean.sh
	rm host
