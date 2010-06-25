package com.GCS.xbee_test;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.AtCommand;
import com.rapplogic.xbee.api.AtCommandResponse;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeRequest;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetRxResponse;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.api.zigbee.ZNetTxStatusResponse;

public class XBeeReadTest {

	private static XBee xbee;
	
	private static final int CONSTANT = 123;		// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;	// packet payload size of 84 bytes (32-bit ints)
	private static final int NUM_PACKETS = 1000;	// number of packets to send
	private static final int DELAY = 0;			// milliseconds of delay between packet transmits
	private static final String FILE_NAME = "/home/varun/latency.csv";	// CSV of individual packet latencies
	private static final XBeeAddress64 DEST_64 = new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xED);	// IEEE address of receiving node

	/*
	public static void main(String[] args) throws IOException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		int ints[] = new int[PKT_SIZE_INTS];
		BufferedWriter out = null;
			
		int errorCount = 0;
		int packetCount = 0;
		long totalLatency = 0;
		int totalPackets = 0;
		
		// create new latency.csv
		File f = new File(FILE_NAME);
		if (f.exists()) {
			f.delete();
		}
		if (!f.createNewFile()) throw new IOException("Could not create file");
		
		try {
			xbee.open("/dev/ttyUSB0", 115200);
			try {
				out = new BufferedWriter(new FileWriter (FILE_NAME));
				while (packetCount < 1000) {
					// get packet and calculate latency (includes delay)
					long prev = System.currentTimeMillis();
					XBeeResponse resp = xbee.getResponse();
					long next = System.currentTimeMillis();
					packetCount++;
					
					if (resp.getApiId() == ApiId.ZNET_RX_RESPONSE) {	
						byte bytes[] = new byte[((ZNetRxResponse)resp).getData().length];
						for (int i = 0; i < bytes.length; i++)
							bytes[i] = (byte) ((ZNetRxResponse)resp).getData()[i];
						
						// flip endian and save packet as array of 32-bit ints
						for (int i = 0; i < bytes.length; i += 4) {
							ByteBuffer bb = ByteBuffer.allocate(32);
							bb.order(ByteOrder.BIG_ENDIAN);
							bb.put(bytes, i, 4);
							bb.order(ByteOrder.LITTLE_ENDIAN);
							//System.out.print(bb.getInt(0)+", ");
							ints[i / 4] = bb.getInt(0);
						}
						
						// check for errors in packet
						errorCount += errorCheck(ints, packetCount);
						// output
						long latency = next-prev;
						if (latency < 1000) {	// ignore waiting for Arduino to start
							totalLatency += latency;
							totalPackets++;
						}
						//System.out.println("Packet " + packetCount + ": Latency of " + latency + " ms.");
						out.write(packetCount+","+latency+"\n");
					}
				}
				System.out.println("Delay: "+ints[1]+"ms");
				System.out.println("RSSI: "+ getRSSI()+"dBm");
				System.out.println("Errors: " + errorCount + " packets");
				System.out.println("Goodput: " + ((float)(totalPackets*PKT_SIZE_INTS*4*8)/(float)totalLatency) +"kbps");
				System.out.println("Throughput: " + ((float)(totalPackets*128*8)/(float)totalLatency) + "kbps");
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
	*/
	
	public static void main(String[] args) throws IOException, InterruptedException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		int[] payload = new int[PKT_SIZE_INTS*4];	// payload[] entries are actually bytes, but XBee-API doesn't like that primitive...
		BufferedWriter out = null;
			
		int packetCount = 1;
		int errorCount = 0;
		long startTime = 0;

		// create new latency.csv
		File f = new File(FILE_NAME);
		if (f.exists()) {
			f.delete();
		}
		if (!f.createNewFile()) throw new IOException("Could not create file");
		
		// initialize packet payload (payload[0-1] holds packet count)
		payload[0] = 0; payload[1] = 1;
		for (int i = 2; i < PKT_SIZE_INTS*4; i++) payload[i] = CONSTANT;
		
		try {
			xbee.open("/dev/ttyUSB0", 115200);
			try {
				out = new BufferedWriter(new FileWriter (FILE_NAME));
				
				while (packetCount <= NUM_PACKETS) {
					// delay before sending next packet
					if (packetCount > 1) Thread.sleep(DELAY);
					// 1st packet is always slow, skip counting it
					if (packetCount == 2) startTime  = System.currentTimeMillis();
					
					// send packet and calculate latency after receiving ACK
					long beforeSend = System.nanoTime();
					try {
						xbee.sendSynchronous(new ZNetTxRequest(DEST_64, payload), 5000);
						//xbee.sendAsynchronous(new ZNetTxRequest(DEST_64, payload));
					} catch (XBeeTimeoutException e) {
						errorCount++;
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
				System.out.println("RSSI:\t\t"+ getRSSI()+"dBm");
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
	
	/*
	private static int errorCheck(int[] ints, int packetCount) {
		if (ints[0] != packetCount) {
			System.err.println("count inconsistency, count @ "+packetCount+", received: "+ints[0]);
			return 1;
		}
		for (int i = 2; i < PKT_SIZE_INTS; i++) {
			if (ints[i] != CONSTANT) {
				System.err.println("Constant inconsistency at int #" + i + ", value is " + ints[i]);
				return 1;
			}
		}
		return 0;
	}
	*/

	private static int getRSSI() {
		try {
			AtCommandResponse response = (AtCommandResponse) xbee.sendSynchronous(new AtCommand("DB"), 5000);
			if (response.isOk()) {
				return -1*response.getValue()[0];
			}
		} catch (XBeeTimeoutException e) {
			e.printStackTrace();
		} catch (XBeeException e) {
			e.printStackTrace();
		}
		return 1;
	}

}
