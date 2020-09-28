print("===========================")
print("reading the TCP PAYLOAD PROGRAM")
print("===========================")
from scapy.all import *
import binascii
try:
    pkt_list = rdpcap("test.")

    print("First way:")
    packet_list = []
    for pkt in pkt_list:
        if Raw in pkt:
            print("packet payload description:")
            data = pkt[Raw].load
            data = binascii.hexlify(data)
            print(data)



            packet_list.append(pkt)

    wrpcap("new.pcap", packet_list)
except FileNotFoundError:
    print("no :))")
