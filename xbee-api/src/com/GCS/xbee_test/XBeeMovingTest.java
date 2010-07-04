package com.GCS.xbee_test;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

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

public class XBeeMovingTest {
	// configuration flags
	private static boolean UART_ACK = false;			// receive UART ACK instead of network ACK
	private static boolean DISCOVERY = true;			// force path discovery on each transmit

	
	// our wonderfully useful constants
	private static final int CONSTANT = 123;			// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;		// packet payload size of 84 bytes (32-bit ints)

	private static final XBeeAddress64 DEST_64 = new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xED);	// IEEE address of receiving node (API)
	/*
	private static final XBeeAddress64 DEST_64 = (ROUTER) ? 
			new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xE6) :	// IEEE address of receiving node (API)
			new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xEE) ;	// IEEE address of receiving node (AT)
	 */
	
	// fields
	private XBee xbee;
	private final static Logger log = Logger.getLogger(XBeeMovingTest.class);
	private int packetCount;
	private int errorCount;
	private int ACKcount;			// number of ACK packets received
	private long beforeSend;
	private int totalLatency;
	private int[] payload;
	private XBeeAddress16 dest_16;
	
	public XBeeMovingTest() {
		initFields();
		try {
			initXBee();
		} catch (XBeeException e) {
			e.printStackTrace();
			System.exit(-1);
		}
		initGUI();
	}
	
	private void initFields() {
		packetCount = 0;
		errorCount = 0;
		ACKcount = 0;
		beforeSend = 0;
		totalLatency = 0;
		// initialize little endian packet payload (payload[0-3] holds packet count)
		payload = new int[PKT_SIZE_INTS*4];	// payload[] entries are actually bytes, but XBee-API doesn't like that primitive...
		payload[0] = 1;
		for (int i = 4; i < PKT_SIZE_INTS*4; i+=4) payload[i] = CONSTANT;
	}
	
	// initialize XBee communication and get 16-bit address of destination
	private void initXBee() throws XBeeException {
		xbee = new XBee();
		xbee.addPacketListener(new ACKPacketListener());
		xbee.open("/dev/ttyUSB0", 115200);
		dest_16 = (DISCOVERY) ? XBeeAddress16.ZNET_BROADCAST : Shared.get16Addr(xbee, DEST_64);
		if (dest_16 == null) {
			log.warn("Could not get 16-bit address");
			dest_16 = XBeeAddress16.ZNET_BROADCAST;
		}
		log.debug("16-bit destination address: " + dest_16);
	}
	
	private void initGUI() {
		JFrame frame = new JFrame("XBee Moving Test");
		JButton sendButton = new JButton("Send!");
		sendButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				sendPacket();
			}
		});
		
		JButton exitButton = new JButton("Exit");
		exitButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				exit();
			}
		});

		JPanel panel = new JPanel(new GridLayout());
		panel.add(sendButton);
		panel.add(exitButton);

		frame.add(panel);
		//this.pack();
		frame.setSize(600, 480);
		frame.setVisible(true);
	}
	
	void exit() {
		errorCount += packetCount - ACKcount;

		log.info("Packets sent:\t" + packetCount);
		log.info("Errors:\t\t" + errorCount + " packets for " + (float)errorCount/(float)packetCount * 100 + "% error");
		log.info("RSSI:\t\t"+ Shared.getRSSI(xbee)+"dBm");
		log.info("Goodput:\t" + ((float)(packetCount*PKT_SIZE_INTS*4*8)/(float)totalLatency*1000000) +"kbps");
		log.info("Throughput:\t" + ((float)(packetCount*128*8)/(float)totalLatency*1000000) + "kbps");
		xbee.close();
		System.exit(0);
	}
	
	void sendPacket() {
		// send packet and calculate latency after receiving ACK
		beforeSend = System.nanoTime();
		try {
			xbee.sendAsynchronous(new ZNetTxRequest(1, DEST_64, dest_16, 0, (UART_ACK) ? 1 : 0, payload));
		} catch (XBeeTimeoutException e) {	
			errorCount++;
			log.error("ERROR, ACK 5sec timeout");
		} catch (XBeeException e) {
			e.printStackTrace();
		}

		//update packet count
		packetCount++;
		payload[3] = (packetCount >> 24) & 0xFF;
		payload[2] = (packetCount >> 16) & 0xFF;
		payload[1] = (packetCount >> 8) & 0xFF;
		payload[0] = packetCount & 0xFF;
	}
	
	public static void main(String args[]) {
		PropertyConfigurator.configure("log4j.properties");
		new XBeeMovingTest();
	}
	
	private class ACKPacketListener implements PacketListener {
		public void processResponse(XBeeResponse response) {
			log.debug(response.getApiId());
			if (response instanceof XBeeFrameIdResponse) {
				int frameID = ((XBeeFrameIdResponse)response).getFrameId();
				log.debug("ACK: frame ID of " + frameID);
				if (response.getApiId() == ApiId.ZNET_TX_STATUS_RESPONSE && response instanceof ZNetTxStatusResponse) {
					ZNetTxStatusResponse rx = (ZNetTxStatusResponse) response;
					log.debug("ACK Discovery Status: " + rx.getDiscoveryStatus());
					log.debug("ACK Delivery Status: " + rx.getDeliveryStatus());
					log.debug("Retry count for node " + rx.getRemoteAddress16() + ": " + rx.getRetryCount());
					if (rx.isSuccess()) {
						ACKcount++;
						long latency = System.nanoTime() - beforeSend;
						totalLatency += latency;

						// output individual packet results
						log.debug("Packet " + packetCount + ": Latency of " + Math.round(latency/1000000) + " ms.");
						log.info(packetCount+","+latency);
					}
				}
			}	
		}
	}
}