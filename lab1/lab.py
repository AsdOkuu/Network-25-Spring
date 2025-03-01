from scapy.all import *


################################################################################
# Part 1. sniffing and Ethernet parsing 
################################################################################

Trace1 = "http.pcap"


from threading import Thread
from time import sleep
import requests


def MySniff():
    print("sniffing start")
    #################################
    ###### start of your code #######
    #################################
    # Sniff for 5 secs
    packets = sniff(timeout=5)
    wrpcap(Trace1, packets)
    #################################
    ###### end of your code #########
    #################################
    print("sniffing stop")

    
def MyHttp():
    print("http start")
    x = requests.get('http://gaia.cs.umass.edu/wireshark-labs/HTTP-ethereal-lab-file3.html')
    print(x.status_code)
    sleep(1)
    print("http stop")


def Q1():
    t1 = Thread(target=MySniff)
    t2 = Thread(target=MyHttp)
    t1.start()
    sleep(1)
    t2.start()
    t2.join()
    t1.join()
    return "done"


def Q2():
    src_mac= ""
    dst_mac = ""
    #################################
    ###### start of your code #######
    #################################
    # Read pcap
    packets = rdpcap(Trace1)
    # Iterate all packets
    for p in packets:
        if TCP in p and p[TCP].dport == 80:
            src_mac = p[Ether].src
            dst_mac = p[Ether].dst
            break
    #################################
    ###### end of your code #########
    #################################
    return src_mac, dst_mac
    


def Q3():    
    theType = 0
    theProto = 0    
    #################################
    ###### start of your code #######
    #################################
    timestamp = 0
    # Read pcap
    packets = rdpcap(Trace1)
    # Iteerate all packets
    for p in packets:
        if TCP in p and p[TCP].dport == 80:
            theType = p[Ether].type
            theProto = p[IP].proto
            timestamp = p.time
            break
    #################################
    ###### end of your code #########
    #################################
    return theType, theProto, timestamp
            


################################################################################
# Part 2. Trace analysis 
################################################################################

Trace2='univ.pcap'

def Q4():
    theLength = 0
    #################################
    ###### start of your code #######
    #################################
    packets = rdpcap(Trace2)
    theLength = len(packets)
    #################################
    ###### end of your code #########
    #################################
    return theLength

def Q5():
    num_tcp = 0
    num_udp = 0
    num_ip = 0
    #################################
    ###### start of your code #######
    #################################
    packets = rdpcap(Trace2)
    # Iterate all packets
    for p in packets:
        if TCP in p:
            num_tcp += 1
        if UDP in p:
            num_udp += 1
        if IP in p:
            num_ip += 1
    #################################
    ###### end of your code #########
    #################################
    return num_ip, num_tcp, num_udp

def Q6():
    flows = set()
    #################################
    ###### start of your code #######
    #################################
    packets = rdpcap(Trace2)
    # Filter all TCP
    tcp_packets = [p for p in packets if TCP in p]
    for p in tcp_packets:
        n0 = (p[IP].src, p[TCP].sport)
        n1 = (p[IP].dst, p[TCP].dport)
        s0 = (n0, n1)
        s1 = (n1, n0)
        # Both situation
        if s0 not in flows and s1 not in flows:
            flows.add(s0)
    #################################
    ###### end of your code #########
    #################################
    return len(flows)

def Q7():
    min_length = 0
    max_length = 0
    median_length = 0
    #################################
    ###### start of your code #######
    #################################
    packets = rdpcap(Trace2)
    # Filter all IP
    ip_packets = [p for p in packets if IP in p]
    # Sort with ans
    lens = sorted(list(map(lambda p: p[IP].len + 18, ip_packets)))
    n = len(ip_packets)
    min_length = lens[0]
    max_length = lens[-1]
    # Calc median
    if n % 2 == 1:
        median_length = lens[n // 2]
    else:
        median_length = (lens[n // 2 - 1] + lens[n // 2]) / 2
    #################################
    ###### end of your code #########
    #################################
    return min_length, median_length, max_length

