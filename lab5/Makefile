# Makefile for C project
CC = gcc
CFLAGS = -Wall -Wextra 
LDFLAGS = 
TARGET = transport
SRCS = main.c lab.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.txt


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

setup_topo:
	./setup_star.sh setup
	
clean_topo:
	./setup_star.sh clean

test: $(TARGET)
	# Clean up previous test files
	rm -f output1.txt output2.txt
	docker exec host4 rm -f /file1.txt /file2.txt || true
	docker exec host1 rm -f /log.txt || true
	docker exec host2 rm -f /log.txt || true

	# Copy program to all host containers
	docker cp $(TARGET) host1:/$(TARGET)
	docker cp $(TARGET) host2:/$(TARGET)
	docker cp $(TARGET) host4:/$(TARGET)
	
	# Kill any previous instances of the program
	docker exec -it host1 pkill -f $(TARGET) || true
	docker exec -it host2 pkill -f $(TARGET) || true
	docker exec -it host4 pkill -f $(TARGET) || true

	# set packet loss to be 0.1
	# docker exec -it device tc qdisc add dev device-eth3  root netem loss 10%
	# cancel packet loss
	#docker exec -it device tc qdisc del dev device-eth3 root 

	# Run program in host1 and host2 as sender and log output inside container
	docker exec -dit host1 bash -c "/$(TARGET) sender > /log.txt 2>&1"
	docker exec -dit host2 bash -c "/$(TARGET) sender > /log.txt 2>&1"

	# Run program in host4 as receiver  
	docker exec -it host4 bash -c "/$(TARGET) receiver > /log.txt 2>&1"

	# Wait for a few seconds to allow the program to run
	sleep 1
	

	@echo "=========================="
	@echo "Check result correctness:"
	@echo "=========================="
	# Copy source files from host1 and host2
	docker cp host1:/file1.txt ./file1.txt
	docker cp host2:/file2.txt ./file2.txt

	# Copy result files from host4
	docker cp host4:/file1.txt ./output1.txt
	docker cp host4:/file2.txt ./output2.txt
	
	# Compare files
	@echo "---------------------------"
	@if diff -q file1.txt output1.txt > /dev/null; then \
		echo "file1.txt and output1.txt: succeed"; \
	else \
		echo "file1.txt and output1.txt: fail"; \
	fi
	@if diff -q file2.txt output2.txt > /dev/null; then \
		echo "file2.txt and output2.txt: succeed"; \
	else \
		echo "file2.txt and output2.txt: fail"; \
	fi

	@echo "=========================="
	@echo "Check throughput."
	@echo "=========================="
	docker exec -it host1 cat /log.txt | grep "throughput" 
	docker exec -it host2 cat /log.txt | grep "throughput" 
	
	
	