package com.GCS.xbee_test;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.zigbee.ZNetRxResponse;

public class XBeeSendTestPtP_R {

	private static XBee xbee;
	private final static Logger log = Logger.getLogger(XBeeSendTestPtP_R.class);

	private static final int CONSTANT = 123;		// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;	// packet payload size of 84 bytes (32-bit ints)
	private static final int NUM_PACKETS = 1000;	// number of packets to send

	public static void main(String[] args) throws InterruptedException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();

		int packetCount = 0;
		int errorCount = 0;

		try {
			xbee.open("/dev/ttyUSB0", 115200);

			while (packetCount < NUM_PACKETS) {
				// receive packet and calculate single packet receive latency
				long beforeReceive = System.nanoTime();
				XBeeResponse resp = xbee.getResponse();
				long latency = System.nanoTime() - beforeReceive;
				
				if (resp.getApiId() == ApiId.ZNET_RX_RESPONSE) {	
					log.info("Packet " + packetCount + ": Latency of " + latency/1000000 + " ms.");
					// check for errors
					ZNetRxResponse rx = (ZNetRxResponse) resp;
					if (rx.getData().length == PKT_SIZE_INTS * 4) {
						errorCount += errorCheck(rx, packetCount);
						packetCount++;
					}
					else
						log.error("ERROR in packet data length");
				}
			}
		}
		catch (XBeeException e) {
			e.printStackTrace();
		}
		finally {
			xbee.close();
		}

	}
	
	private static int errorCheck(ZNetRxResponse resp, int packetCount) {
		int[] payload = new int[PKT_SIZE_INTS];
		byte[] bytes = new byte[PKT_SIZE_INTS*4];
		// copy packet data into an array of bytes (each element in data is a byte)
		for (int i = 0; i < bytes.length; i++)
			bytes[i] = (byte)resp.getData()[i];
		// convert big endian bytes into a payload of 32-bit ints
		for (int i = 0; i < bytes.length; i += 4) {
			ByteBuffer bb = ByteBuffer.allocate(32);
			bb.order(ByteOrder.BIG_ENDIAN);
			bb.put(bytes, i, 4);
			payload[i/4] = bb.getInt(0);
		}
		// check if first int contains the right count
		if (payload[0] != packetCount) {
			log.warn("count inconsistency, count @ "+packetCount+", received: "+payload[0]);
			return 1;
		}
		
		// check if the rest of the ints contains the CONSTANT
		for (int i = 1; i < PKT_SIZE_INTS; i++) {
			if (payload[i] != CONSTANT) {
				log.warn("Constant inconsistency at int #" + i + ", value is " + payload[i]);
				return 1;
			}
		}
		return 0;
	}


}
