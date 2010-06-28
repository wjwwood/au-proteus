package com.GCS.xbee_test;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;

import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;

public class XBeeSendTestPtP {

	private static XBee xbee;
	
	private static final int CONSTANT = 123;		// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;	// packet payload size of 84 bytes (32-bit ints)
	private static final int NUM_PACKETS = 200;	// number of packets to send
	private static final int DELAY = 0;			// milliseconds of delay between packet transmits
	private static final String FILE_NAME = "/home/varun/latency.csv";	// CSV of individual packet latencies
	private static final XBeeAddress64 DEST_64 = new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xED);	// IEEE address of receiving node

	public static void main(String[] args) throws InterruptedException, IOException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		int[] payload = new int[PKT_SIZE_INTS*4];	// payload[] entries are actually bytes, but XBee-API doesn't like that primitive...
		BufferedWriter out = null;
			
		int packetCount = 1;
		int errorCount = 0;
		long startTime = 0;

		// create new latency.csv
		Shared.createFile(FILE_NAME);
		
		// initialize packet payload (payload[0-1] holds packet count)
		payload[0] = 0; payload[1] = 1;
		for (int i = 2; i < PKT_SIZE_INTS*4; i++) payload[i] = CONSTANT;
		
		try {
			xbee.open("/dev/ttyUSB0", 115200);
			try {
				out = new BufferedWriter(new FileWriter (FILE_NAME));
				
				XBeeAddress16 dest_16 = Shared.get16Addr(xbee, DEST_64);
				
				while (packetCount <= NUM_PACKETS) {
					// delay before sending next packet
					if (packetCount > 1) Thread.sleep(DELAY);
					// 1st packet is always slow, skip counting it
					if (packetCount == 2) startTime  = System.currentTimeMillis();
					
					// send packet and calculate latency after receiving ACK
					long beforeSend = System.nanoTime();
					try {
						//xbee.sendSynchronous(new ZNetTxRequest(1, DEST_64, dest_16, 0, 1, payload), 5000);
						xbee.sendSynchronous(new ZNetTxRequest(DEST_64, payload), 5000);
						//xbee.sendAsynchronous(new ZNetTxRequest(DEST_64, payload));
					} catch (XBeeTimeoutException e) {
						errorCount++;
						System.out.println("ERROR");
					}
					long latency = System.nanoTime() - beforeSend;
					
					// output individual packet results
					//System.out.println("Packet " + packetCount + ": Latency of " + latency/1000000 + " ms.");
					out.write(packetCount+","+latency+"\n");
					
					//update packet count
					packetCount++;
					payload[0] = (packetCount >> 8) & 0xFF;
					payload[1] = packetCount & 0xFF;
				}
				
				long totalLatency = System.currentTimeMillis() - startTime;
				int totalPackets = NUM_PACKETS - errorCount - 1;	// -1 for not counting 1st packet
				
				System.out.println("Delay:\t\t"+DELAY+"ms");
				System.out.println("Errors:\t\t" + errorCount + " packets");
				System.out.println("RSSI:\t\t"+ Shared.getRSSI(xbee)+"dBm");
				System.out.println("Goodput:\t" + ((float)(totalPackets*PKT_SIZE_INTS*4*8)/(float)totalLatency) +"kbps");
				System.out.println("Throughput:\t" + ((float)(totalPackets*128*8)/(float)totalLatency) + "kbps");
			}
			catch (IOException e) {
				e.printStackTrace();
			}
			finally {
				if (out != null) out.close();
			}
		}
		catch (XBeeException e) {
			e.printStackTrace();
		}
		finally {
			xbee.close();
		}

	}


}
