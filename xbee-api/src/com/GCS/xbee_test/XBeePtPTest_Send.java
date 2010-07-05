package com.GCS.xbee_test;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.PacketListener;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeFrameIdResponse;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.api.zigbee.ZNetTxStatusResponse;

public class XBeePtPTest_Send {

	// globals
	private static XBee xbee;
	private final static Logger log = Logger.getLogger(XBeePtPTest_Send.class);
	private static int ACKcount;			// number of ACK packets received

	// configuration flags
	private static boolean SYNC = true;					// use synchronous transmit (wait for ACK before next transmit)
	private static boolean UART_ACK = false;			// receive UART ACK instead of network ACK
	private static boolean DISCOVERY = true;			// force path discovery on each transmit
	private static boolean API = true;					// firmware of destination (API or AT)
	
	// our wonderfully useful constants
	private static final int CONSTANT = 123;			// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;		// packet payload size of 84 bytes (32-bit ints)
	private static final int NUM_PACKETS = 1000;		// number of packets to send
	private static final int DELAY = 20;				// milliseconds of delay between packet transmits
	private static final XBeeAddress64 DEST_64 = (API) ? 
					new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xED) :	// IEEE address of receiving node (API)
					new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xEE) ;	// IEEE address of receiving node (AT)
	
	public static void main(String[] args) throws InterruptedException {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		int[] payload = new int[PKT_SIZE_INTS*4];	// payload[] entries are actually bytes, but XBee-API doesn't like that primitive...

		int packetCount = 1;
		int errorCount = 0;
		long startTime = 0;
		ACKcount = 0;

		// initialize little endian packet payload (payload[0-3] holds packet count)
		payload[0] = 1;
		for (int i = 4; i < PKT_SIZE_INTS*4; i+=4) payload[i] = CONSTANT;

		try {
			xbee.open("/dev/ttyUSB0", 115200);
			XBeeAddress16 dest_16 = (DISCOVERY) ? XBeeAddress16.ZNET_BROADCAST : Shared.get16Addr(xbee, DEST_64);
			if (dest_16 == null) {
				log.error("Could not get 16-bit address");
				dest_16 = XBeeAddress16.ZNET_BROADCAST;
			}
			// add a packet listener for ACKs
			ACKPacketListener ACKListener = new ACKPacketListener();
			xbee.addPacketListener(ACKListener);

			while (packetCount <= NUM_PACKETS) {
				// delay before sending next packet
				if (!SYNC) Thread.sleep(DELAY);
				// 1st packet is always slow, skip counting it
				if (packetCount == 2) startTime  = System.currentTimeMillis();

				// send packet and calculate latency after receiving ACK
				long beforeSend = System.nanoTime();
				try {
					if (SYNC)
						xbee.sendSynchronous(new ZNetTxRequest(1, DEST_64, dest_16, 0, (UART_ACK) ? 1 : 0, payload), 5000);
					else
						xbee.sendAsynchronous(new ZNetTxRequest(1, DEST_64, dest_16, 0, (UART_ACK) ? 1 : 0, payload));
				} catch (XBeeTimeoutException e) {
					errorCount++;
					log.error("ERROR, ACK 5sec timeout");
				}
				long latency = System.nanoTime() - beforeSend;

				// output individual packet results
				log.debug("Packet " + packetCount + ": Latency of " + Math.round(latency/1000000) + " ms.");
				log.info(packetCount+","+latency);

				//update packet count
				packetCount++;
				payload[3] = (packetCount >> 24) & 0xFF;
				payload[2] = (packetCount >> 16) & 0xFF;
				payload[1] = (packetCount >> 8) & 0xFF;
				payload[0] = packetCount & 0xFF;
			}

			long totalLatency = System.currentTimeMillis() - startTime;
			int totalPackets = NUM_PACKETS - errorCount - 1;	// -1 for not counting 1st packet
			
			// wait for some lagging ACKs and total up missing ACKs
			Thread.sleep(500);
			errorCount += NUM_PACKETS - ACKcount;
			xbee.removePacketListener(ACKListener);

			log.info("Delay:\t\t"+((SYNC) ? "sync " : DELAY+"ms"));
			log.info("Errors:\t\t" + errorCount + " packets for " + (float)errorCount/(float)NUM_PACKETS * 100 + "% error");
			log.info("RSSI:\t\t"+ Shared.getRSSI(xbee)+"dBm");
			log.info("Goodput:\t" + ((float)(totalPackets*PKT_SIZE_INTS*4*8)/(float)totalLatency) +"kbps");
			log.info("Throughput:\t" + ((float)(totalPackets*128*8)/(float)totalLatency) + "kbps");
		}
		catch (XBeeException e) {
			e.printStackTrace();
		}
		finally {
			xbee.close();
		}
	}
	
	// Packet listener class
	private static class ACKPacketListener implements PacketListener {
		public void processResponse(XBeeResponse response) {
			if (response instanceof XBeeFrameIdResponse) {
				int frameID = ((XBeeFrameIdResponse)response).getFrameId();
				log.debug("ACK: frame ID of " + frameID);
				if (response.getApiId() == ApiId.ZNET_TX_STATUS_RESPONSE && response instanceof ZNetTxStatusResponse) {
					ZNetTxStatusResponse rx = (ZNetTxStatusResponse) response;
					log.debug("ACK Discovery Status: " + rx.getDiscoveryStatus());
					log.debug("ACK Delivery Status: " + rx.getDeliveryStatus());
					log.debug("Retry count for node " + rx.getRemoteAddress16() + ": " + rx.getRetryCount());
					if (rx.isSuccess()) ACKcount++;
				}
			}	
		}
	}
}