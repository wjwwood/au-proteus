package com.GCS.xbee_test;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeFrameIdResponse;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.api.zigbee.ZNetTxStatusResponse;

public class XBeeMovingTest extends JFrame {
	private static final long serialVersionUID = 1L;
	// configuration flags
	private static boolean UART_ACK = true;				// receive UART ACK instead of network ACK
	private static boolean DISCOVERY = false;			// force path discovery on each transmit
	private static boolean API = true;					// firmware of destination (API or AT)
	
	// our wonderfully useful constants
	private static final int CONSTANT = 123;			// constant number to fill packet
	private static final int PKT_SIZE_INTS = 21;		// packet payload size of 84 bytes (32-bit ints)
	private static final XBeeAddress64 DEST_64 = (API) ? 
					new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xED) :	// IEEE address of receiving node (API)
					new XBeeAddress64(0, 0x13, 0xA2, 0, 0x40, 0x62, 0xD6, 0xEE) ;	// IEEE address of receiving node (AT)

	// fields
	private XBee xbee;
	private final static Logger log = Logger.getLogger(XBeeMovingTest.class);
	private int packetCount;
	private int errorCount;
	private int ACKcount;			// number of ACK packets received
	private int totalLatency;
	private int[] payload;
	private XBeeAddress16 dest_16;
	
	public XBeeMovingTest() throws XBeeException {
		super ("XBee Moving Test");
		
		// initialize fields
		packetCount = 0;
		errorCount = 0;
		ACKcount = 0;
		totalLatency = 0;
		// initialize little endian packet payload (payload[0-3] holds packet count)
		payload = new int[PKT_SIZE_INTS*4];	// payload[] entries are actually bytes, but XBee-API doesn't like that primitive...
		payload[0] = 1;
		for (int i = 4; i < PKT_SIZE_INTS*4; i+=4) payload[i] = CONSTANT;
		
		// initialize XBee communication and get 16-bit address of destination
		xbee = new XBee();
		xbee.open("/dev/ttyUSB0", 115200);
		XBeeAddress16 dest_16 = (DISCOVERY) ? XBeeAddress16.ZNET_BROADCAST : Shared.get16Addr(xbee, DEST_64);
		if (dest_16 == null) {
			log.error("Could not get 16-bit address");
		}
		
		// initialize GUI
		JButton sendButton = new JButton("Send!");
		sendButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				sendPacket();
			}
		});
		sendButton.setSize(300, 300);
		
		JButton exitButton = new JButton("Exit");
		exitButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent arg0) {
				exit();
			}
		});

		JPanel panel = new JPanel();
		panel.add(sendButton);
		panel.add(exitButton);

		this.add(panel);
		this.pack();
		this.setVisible(true);
	}
	
	void exit() {
		errorCount += packetCount - ACKcount;

		log.info("Errors:\t\t" + errorCount + " packets for " + (float)errorCount/(float)packetCount * 100 + "% error");
		log.info("RSSI:\t\t"+ Shared.getRSSI(xbee)+"dBm");
		log.info("Goodput:\t" + ((float)(packetCount*PKT_SIZE_INTS*4*8)/(float)totalLatency) +"kbps");
		log.info("Throughput:\t" + ((float)(packetCount*128*8)/(float)totalLatency) + "kbps");
		xbee.close();
		System.exit(0);
	}
	
	void sendPacket() {
		// send packet and calculate latency after receiving ACK
		XBeeResponse response = null;
		long beforeSend = System.nanoTime();
		try {
			response = xbee.sendSynchronous(new ZNetTxRequest(1, DEST_64, dest_16, 0, (UART_ACK) ? 1 : 0, payload), 5000);
		} catch (XBeeTimeoutException e) {	
			errorCount++;
			log.error("ERROR, ACK 5sec timeout");
		} catch (XBeeException e) {
			e.printStackTrace();
		}
		long latency = System.nanoTime() - beforeSend;
		totalLatency += latency;

		// output individual packet results
		log.debug("Packet " + packetCount + ": Latency of " + Math.round(latency/1000000) + " ms.");
		log.info(packetCount+","+latency);

		//update packet count
		packetCount++;
		payload[3] = (packetCount >> 24) & 0xFF;
		payload[2] = (packetCount >> 16) & 0xFF;
		payload[1] = (packetCount >> 8) & 0xFF;
		payload[0] = packetCount & 0xFF;
		
		// check out ACK
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
	
	public static void main(String args[]) throws XBeeException {
		PropertyConfigurator.configure("log4j.properties");
		new XBeeMovingTest();
	}
}