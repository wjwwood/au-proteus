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
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetRxResponse;

public class XBeeReadTest {

	private static XBee xbee;
	
	private static final int CONSTANT = 123456;
	private static final int PKT_SIZE_INTS = 21;
	private static final String FILE_NAME = "/home/varun/latency.csv";

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
				out = new BufferedWriter(new FileWriter ("/home/varun/latency.csv"));
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
				System.out.println("Delay: "+ints[1]);
				System.out.println("RSSI: "+ getRSSI());
				System.out.println("Errors: " + errorCount);
				System.out.println("Goodput: " + ((float)(totalPackets*PKT_SIZE_INTS*4*8)/(float)totalLatency) +"kbps");
				System.out.println("Throughput: " + ((float)(totalPackets*128*8)/(float)totalLatency) + "kbps");
			}
			catch (IOException e) {
				e.printStackTrace();
			}
			finally {
				out.close();
			}
		}
		catch (XBeeException e) {
			e.printStackTrace();
		}
		finally {
			xbee.close();
		}

	}
	
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

	private static int getRSSI() {
		AtCommand at = new AtCommand("DB");
		try {
			AtCommandResponse response = (AtCommandResponse) xbee.sendSynchronous(at, 5000);
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
