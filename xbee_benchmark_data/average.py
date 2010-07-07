#!/usr/bin/python

import sys
import re

def calc_latency(log_files):
	"""Calculates the average latency for all entries in all given log files"""
	latency_re = re.compile(r"(\d+),(\d+)$")
	total_count = 0
	total_latency = 0
	
	for log in log_files:
		log.seek(0)
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
				print latency_m.group(1) + "," + latency_m.group(2)
		total_count = total_count + packet_count - first_count + 1
	
	return (float(total_latency) / float(total_count))

def get_delay(log_files):
	"""Gets the delay value from the first log file (should be same in all log files)"""
	delay_re = re.compile(r"Delay:\s+(\d+)")
	log_files[0].seek(0)
	for line in log_files[0]:
		delay_m = delay_re.search(line.strip())
		if delay_m:
			return int(delay_m.group(1))

def calc_error(log_files):
	"""Calculates the average error in all given log files"""
	error_re = re.compile(r"Errors:\s+(\d+)")
	total_error = 0

	for log in log_files:
		log.seek(0)
		for line in log:
			error_m = error_re.search(line.strip())
			if error_m:
				total_error = total_error + int(error_m.group(1))
	return (float(total_error) / float(len(log_files)))

def calc_rssi(log_files):
	"""Calculates the average RSSI in all given log files"""
	rssi_re = re.compile(r"RSSI:\s+(-\d+)")
	total_rssi = 0
	for log in log_files:
		log.seek(0)
		for line in log:
			rssi_m = rssi_re.search(line.strip())
			if rssi_m:
				total_rssi = total_rssi + int(rssi_m.group(1))
	return (float(total_rssi) / float(len(log_files)))

def calc_throughput(log_files):
	"""Calculates the average throughput in all given log files"""
	thr_re = re.compile(r"Throughput:\s+(\d+\.\d+)")
	total_thr = 0.0
	for log in log_files:
		log.seek(0)
		for line in log:
			thr_m = thr_re.search(line.strip())
			if thr_m:
				total_thr = total_thr + float(thr_m.group(1))
	return (total_thr / float(len(log_files)))

def main(argv=None):
	if argv is None:
		argv = sys.argv
	log_files = [open(filename) for filename in argv[1:]]

	print "Average latency: " + str(calc_latency(log_files)/1000000) + " ms"
	print "Delay: " + str(get_delay(log_files)) + " ms"
	print "Average error: " + str(calc_error(log_files))
	print "Average RSSI: " + str(calc_rssi(log_files)) + "dBm"
	print "Average Throughput: " + str(calc_throughput(log_files)) + "kbps"
	for log in log_files:
		log.close()

if __name__ == "__main__":
	main()
