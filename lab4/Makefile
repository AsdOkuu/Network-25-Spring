# Makefile for lab4
CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = device
SRCS = main.c lab.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpcap

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)


setup_env:	
	@echo "Setting up node..."
	apt install libpcap-dev
	cp daemon.json /etc/docker/daemon.json
	systemctl daemon-reload
	systemctl stop docker.service
	systemctl stop docker.socket
	systemctl start docker.service
	systemctl start docker.socket
	docker image build -t node .
	@echo "Node setup complete."


setup_star:
	./setup_star.sh setup
clean_star:
	./setup_star.sh clean



test_hub: 
	docker cp $(TARGET) device:/$(TARGET)
	docker exec -dit device bash -c "./$(TARGET) device hub"
	docker exec -dit host2 bash -c "tcpdump -i host2-eth0 -ne -v > test_hub_host2.log"
	sleep 1
	docker exec -it host1 bash -c "ping 10.0.0.4 -c 3 > test_hub_host1.log"
	sleep 1
	docker exec device bash -c "pkill $(TARGET)"
	docker exec host2 bash -c "pkill tcpdump"
	docker cp host2:/test_hub_host2.log .
	docker cp host1:/test_hub_host1.log . 
	@echo "================================================"
	@echo "Display test_hub_host2.log, a correct log should have 6 records"
	@echo "-------------------------------------------------"
	cat test_hub_host2.log
	@echo "================================================"
	echo "Display test_hub_host1.log, a correct log should have 3 records"
	@echo "-------------------------------------------------"
	cat test_hub_host1.log
	@echo "================================================"

test_switch: 
	docker cp $(TARGET) device:/$(TARGET)
	docker exec -dit device bash -c "./$(TARGET) device switch"
	docker exec -dit host2 bash -c "tcpdump -i host2-eth0 -ne -v > test_switch_host2.log"
	sleep 1
	docker exec -it host1 bash -c "ping 10.0.0.4 -c 3 > test_switch_host1.log"
	sleep 1
	docker exec device bash -c "pkill $(TARGET)"
	docker exec host2 bash -c "pkill tcpdump"
	docker cp host2:/test_switch_host2.log .
	docker cp host1:/test_switch_host1.log . 
	@echo "================================================"
	@echo "Display test_switch_host2.log, a correct log should have 1 record"
	@echo "-------------------------------------------------"
	cat test_switch_host2.log
	@echo "================================================"
	echo "Display test_switch_host1.log, a correct log should have 3 records"
	@echo "-------------------------------------------------"
	cat test_switch_host1.log
	@echo "================================================"

setup_chain:
	./setup_chain.sh setup
clean_chain:	
	./setup_chain.sh clean

	
test_router:
	docker cp $(TARGET) device1:/$(TARGET)
	docker cp $(TARGET) device2:/$(TARGET)
	docker cp $(TARGET) device3:/$(TARGET)
	docker cp $(TARGET) device4:/$(TARGET)
	docker exec -dit device1 bash -c "./$(TARGET) device1 router"
	docker exec -dit device2 bash -c "./$(TARGET) device2 router"
	docker exec -dit device3 bash -c "./$(TARGET) device3 router"
	docker exec -dit device4 bash -c "./$(TARGET) device4 router"
	docker exec -it host1 bash -c "ping 10.0.5.2 -c 5"
	sleep 1
	docker exec device1 bash -c "killall $(TARGET)"
	docker exec device2 bash -c "killall $(TARGET)"
	docker exec device3 bash -c "killall $(TARGET)"
	docker exec device4 bash -c "killall $(TARGET)"
	@echo "================================================"
	@echo "It takes 3 seconds to propagate routing entry to all routers."
	@echo "So about 3 pings should fail, and the remaining should work."
	

.PHONY: all clean
