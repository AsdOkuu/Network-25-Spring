all: 
	gcc udp.c -o transport
	docker container cp transport n1:.
	docker container cp transport n2:.
	docker exec -dit n2 bash -c "./transport server 10.0.0.2 8080 > /log.txt 2>&1 "
	sleep 1
	docker exec -it n1 bash -c "./transport client 10.0.0.2 8080 > /log.txt 2>&1 "
	sleep 1
	docker exec -it n1 killall transport || true
	docker exec -it n2 killall transport || true
	
	@echo "======================"
	@echo "n1 log:"
	@echo "======================"
	docker exec -it n1 cat /log.txt

	@echo "======================"
	@echo "n2 log:"
	@echo "======================"
	docker exec -it n2 cat /log.txt


setup:
	./setup.sh

clean:
	./clean.sh
	rm transport
