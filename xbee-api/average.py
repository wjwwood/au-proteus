#!/usr/bin/python

import sys
import re

def calc_latency(log_file):
	total_latency = 0
	first_count = 0
	packet_count = 0
	read_first = False
	
	for line in log_file:
		data_m = data_re.search(line.strip())
		if data_m:
			packet_count = int(data_m.group(1))
			packet_latency = int(data_m.group(2))
			if not read_first:
				first_count = packet_count
				read_first = True
			total_latency = total_latency + packet_latency
			print "Packet " + str(packet_count) + ": Latency of " + str(packet_latency)
	
	return (float(total_latency) / float(packet_count - first_count + 1))

try:
	filename = sys.argv[1]
except IndexError:
	filename = raw_input("Enter filename: ")

data_re = re.compile(r"(\d+),(\d+)$")
log = open(filename)

print "Average latency: " + str(calc_latency(log)/1000000) + " ms."
log.close()
