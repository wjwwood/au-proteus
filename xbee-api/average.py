#!/usr/bin/python

import sys
import re

def calc_latency(log_files):
	"""Calculates the average latency for all entries in all given log files"""
	latency_re = re.compile(r"(\d+),(\d+)$")
	total_count = 0
	total_latency = 0
	
	for log in log_files:
		first_count = 0
		packet_count = 0
		read_first = False
		for line in log:
			latency_m = latency_re.search(line.strip())
			if latency_m:
				packet_count = int(latency_m.group(1))
				packet_latency = int(latency_m.group(2))
				if not read_first:
					first_count = packet_count
					read_first = True
				total_latency = total_latency + packet_latency
				#print "Packet " + str(packet_count) + ": Latency of " + str(packet_latency)
		total_count = total_count + packet_count - first_count + 1
	
	return (float(total_latency) / float(total_count))

def calc_error(log_files):
	"""Calculates the average error in all given log files"""
	error_re = re.compile(r"Errors:\s+(\d+)")
	total_error = 0

	for log in log_files:
		for line in log:
			error_m = error_re.match(line.strip())
			if error_m:
				print error_m.group(1)
				total_error = total_error + int(error_m.group(1))
	return (float(total_error) / float(len(log_files)))

def calc_rssi(log_files):
	"""Calculates the average RSSI in all given log files"""
	rssi_re = re.compile(r"RSSI:\s+(-\d+)")
	total_rssi = 0
	for log in log_files:
		for line in log:
			rssi_m = rssi_re.search(line.strip())
			if rssi_m:
				total_rssi = total_rssi + int(rssi.group(1))
	return (float(total_rssi) / float(len(log_files)))

def main(argv=None):
	if argv is None:
		argv = sys.argv
	log_files = [open(filename) for filename in argv[1:]]

	print "Average latency: " + str(calc_latency(log_files)/1000000) + " ms."
	print "Average error: " + str(calc_error(log_files))
	print "Average RSSI: " + str(calc_rssi(log_files)) + "dBm"
	for log in log_files:
		log.close()

if __name__ == "__main__":
	main()
