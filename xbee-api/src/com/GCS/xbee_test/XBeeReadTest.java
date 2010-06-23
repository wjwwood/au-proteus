package com.GCS.xbee_test;

import java.io.BufferedWriter;
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

	public static void main(String[] args) throws IOException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		int ints[] = new int[PKT_SIZE_INTS];
		BufferedWriter out = null;
			
		int errorCount = 0;
		int packetCount = 0;
		
		
		try {
			xbee.open("/dev/ttyUSB0", 115200);
			try {
				out = new BufferedWriter(new FileWriter ("/home/varun/latency.csv"));
				while (packetCount < 10) {
					// get packet and calculate latency
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
						System.out.println("Packet " + packetCount + ": Latency of " + (next-prev) + " ms.");
						out.write(packetCount+","+(next-prev)+"\n");
					}
				}
				System.out.println("RSSI: "+getRSSI());
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
		for (int i = 1; i < PKT_SIZE_INTS; i++) {
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
