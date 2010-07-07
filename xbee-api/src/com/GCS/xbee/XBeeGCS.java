/**
 * XBeeGCS.java
 * 
 * The main class for the XBeeGCS.
 * 
 * This class controls XBee communication and reads incoming packets from the ArduPilot.
 * It maintains a GUI for loading an arbitrary waypoint in air, using the CollisionAvoidance class.
 */
package com.GCS.xbee;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.HashMap;
import java.util.InputMismatchException;
import java.util.NoSuchElementException;
import java.util.Scanner;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextField;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.PacketListener;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.zigbee.ZNetRxResponse;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.util.ByteUtils;

public class XBeeGCS {
	
	private final static boolean ENABLE_COLLISION_AVOIDANCE = false;
	
	// class variables
	private final static Logger log = Logger.getLogger(XBeeGCS.class);	// logger for the log4j system
	private static XBee xbee;		// the XBee hooked up to this GCS
	
	// instance variables
	private HashMap<XBeeAddress64, PlaneData> dataMap;		// collection of all telemetry data indexed by IEEE address
	private XBeeAddress64 latest;							// latest plane to transmit valid telemetry data
	private int planeCounter;								// the number of planes

	public XBeeGCS () {
		PropertyConfigurator.configure("log4j.properties");
		xbee = new XBee();
		setDataMap(new HashMap<XBeeAddress64, PlaneData>());
		planeCounter = 0;

		xbee.addPacketListener(new GCSPacketListener());	// packet listener for plane data

		new GUI();	// create a GUI for loading arbitrary waypoints
		
		if (ENABLE_COLLISION_AVOIDANCE)
			new Thread(new CollisionAvoidance(this)).start();
	}
	
	private void setDataMap(HashMap<XBeeAddress64, PlaneData> dataMap) {
		this.dataMap = dataMap;
	}

	/**
	 * Returns the internal hash map of plane telemetry data indexed by each plane's XBee's 64-bit IEEE address.
	 * @return the internal hash map of plane telemetry data
	 */
	public HashMap<XBeeAddress64, PlaneData> getDataMap() {
		return dataMap;
	}
	
	private void setLatest(XBeeAddress64 latest) {
		this.latest = latest;
	}

	/**
	 * Returns the 64-bit IEEE address of the XBee radio belonging to the plane that last transmitted valid telemetry data.
	 * @return the 64-bit IEEE address of the last plane radio to send valid telemetry data
	 */
	public XBeeAddress64 getLatest() {
		return latest;
	}
	
	/**
	 * Updates the internal hash map with new plane data and informs the collision avoidance thread of it.
	 * 
	 * Note: this method should be thread-safe with regards to the collision avoidance algorithm.
	 * 
	 * @param addr	The 64-bit IEEE address of the sending node
	 * @param data	The telemetry data sent by the node
	 */
	void addData(XBeeAddress64 addr, PlaneData data) {
		if (data != null) {
			// associate a plane number with the incoming data
			if (!getDataMap().containsKey(addr))
				data.num = ++planeCounter;
			else 
				data.num = getDataMap().get(addr).num;
			// update hash map
			setLatest(addr);
			log.info(data);
			synchronized (getDataMap()) {
				getDataMap().put(addr, data);
				getDataMap().notify();
			}
		}
	}
	
	/**
	 * Transmits a new waypoint for a plane to follow.
	 * 
	 * This function transmits a waypoint in a big endian GCS_packet_t format specified in the modified
	 * ArduPilot code.  The packet is transmitted synchronously, with an ACK timeout of 5 seconds.
	 * 
	 * @param addr	The 64-bit IEEE address of the node to send the waypoint to
	 * @param wp	The waypoint to send to the node
	 */
	void transmit(XBeeAddress64 addr, Coordinate wp) {
		int waypoint[] = new int[3];
		int payload[] = new int[16];	//4 int32's
		
		// ArduPilot asks for these multipliers
		waypoint[0] = (int) (wp.x * 10); 
		waypoint[1] = (int) (wp.y * 10);
		waypoint[2] = (int) (wp.z * 100);
		
		// flip endian for payload (Arduino is Big Endian)
		for (int i = 0; i < waypoint.length; i++) {
			ByteBuffer bb = ByteBuffer.allocate(32);
			bb.order(ByteOrder.LITTLE_ENDIAN);
			bb.putInt(waypoint[i]);
			bb.order(ByteOrder.BIG_ENDIAN);
			for (int j = 0; j < 4; j++) {
				payload[4*i+j] = bb.array()[j];
			}
		}
		
		// put checksum (sum of lat and alt) in packet payload
		ByteBuffer bb = ByteBuffer.allocate(32);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		bb.putInt(waypoint[0] + waypoint[2]);
		bb.order(ByteOrder.BIG_ENDIAN);
		for (int j = 0; j < 4; j++) {
			payload[12+j] = bb.array()[j];
		}
		
		// send packet
		ZNetTxRequest request = new ZNetTxRequest(addr, payload);
		try {
			xbee.sendSynchronous(request, 5000);
		} catch (XBeeException e) {
			e.printStackTrace();
		}
	
		// log output
		log.debug("zb request is " + request.getXBeePacket().getPacket());
		// undo the multipliers to make logging prettier
		waypoint[0] /= 10; waypoint[1] /= 10; waypoint[2] /= 100;
		log.info("sent " + Arrays.toString(waypoint) + " to Plane " + getDataMap().get(addr).num);
	}
	
	private void exit() {
		xbee.close();
		System.exit(0);
	}

	public static void main(String[] args) throws InterruptedException {
		XBeeGCS gcs = new XBeeGCS();
		// set up the coordinator XBee serial communication
		try {
			xbee.open("/dev/ttyUSB0", 115200);
			// hackish way of making the thread sleep forever.  open to other suggestions...
			Thread.sleep(Long.MAX_VALUE);
		}
		catch (XBeeException e) {
			e.printStackTrace();
		}
		finally {
			gcs.exit();
		}
	}

	/**
	 * Generates a GUI for loading arbitrary waypoints to the latest plane.
	 *
	 */
	private class GUI implements ActionListener {
		private JTextField text;

		public GUI() {
			JFrame frame = new JFrame("Load Arbitrary Waypoint");

			text = new JTextField(50);
			text.addActionListener(this);

			JButton loadButton = new JButton("Load");
			loadButton.addActionListener(this);
			
			JButton exitButton = new JButton("Exit");
			exitButton.addActionListener(new ActionListener() {
				@Override
				public void actionPerformed(ActionEvent arg0) {
					exit();
				}
			});

			JPanel panel = new JPanel();
			panel.add(text);
			panel.add(loadButton);
			panel.add(exitButton);

			frame.add(panel);
			frame.pack();
			frame.setVisible(true);
		}

		/**
		 * Reads coordinate from text field and transmits it to the latest plane
		 */
		@Override
		public void actionPerformed(ActionEvent arg0) {
			String string = text.getText();
			Scanner scanner = new Scanner(string);
			scanner.useDelimiter(", ");
			Coordinate wp = new Coordinate();
			try {
				wp.x = scanner.nextDouble() * 1000000;
				wp.y = scanner.nextDouble() * 1000000;
				wp.z = scanner.nextDouble();
			}
			catch (InputMismatchException e) {
				log.error("GUI: input mismatch exception caught.");
				return;
			}
			catch (NoSuchElementException e) {
				log.error("GUI: 3D point not typed in.");
				return;
			}
			if (getLatest() == null) {
				log.error("GUI: No planes available to transmit to.");
				return;
			}
			else {
				log.info("GUI has initiated transmit of " + wp);
				transmit(getLatest(), wp);
			}
		}

	}

	/**
	 * Packet Listener class for receiving data from XBees on ArduPilots.
	 * Loads new telemetry data into the GCS internal hash map if provided.  Prints all other received packets.
	 * @author varun
	 *
	 */
	private class GCSPacketListener implements PacketListener {

		// keep track of time between packets
		private long next;
		private long prev;

		/**
		 * Receives packet and adds the parsed data from it to the Collision Avoidance object's
		 * internal hash map of plane telemetry data.
		 */
		@Override
		public void processResponse(XBeeResponse response) {
			if (response.getApiId() == ApiId.ZNET_RX_RESPONSE) {
				ZNetRxResponse rx = (ZNetRxResponse) response;

				log.debug("Received RX packet, option is " + rx.getOption() + 
						", sender 64 address is " + ByteUtils.toBase16(rx.getRemoteAddress64().getAddress()) + 
						", remote 16-bit address is " + ByteUtils.toBase16(rx.getRemoteAddress16().getAddress()) + 
						", data is " + ByteUtils.toBase16(rx.getData()));

				addData(rx.getRemoteAddress64(), packetParser(rx));
			}
		}

		/**
		 * Parses the received packet's payload.
		 * 
		 * If the data is not of the specific XBeeGCS payload format, the data is interpreted as a string and logged.
		 * The XBeeGCS format is in the print_position() function of GCS_XBee.pde, and is composed of 10 32-bit big endian integers
		 * 
		 * @param response The ZigBee Receive Packet Frame
		 * @return the PlaneData object containing the new telemetry data.  Returns null if the received packet's payload
		 * is not of XBeeGCS form.
		 */
		private PlaneData packetParser(ZNetRxResponse response) {
			next = System.currentTimeMillis();
			log.debug("Time elapsed is " + (next-prev)+"ms, response length is "+response.getData().length);
			prev = next;	

			// If response isn't a telemetry packet that we sent, just print it out
			if (response.getData().length != 40) {
				StringBuffer sb = new StringBuffer();
				for (int i = 0; i < response.getData().length; i++)
					sb.append((char)response.getData()[i]);
				log.info(sb.toString());
				return null;
			}

			// flip data in packet to little endian and store it in an int array
			int planeDataArray[] = new int[10];
			byte data[] = new byte[response.getData().length];
			for (int i = 0; i < data.length; i++)
				data[i] = (byte) response.getData()[i];
			for (int i = 0; i < data.length; i += 4) {
				ByteBuffer bb = ByteBuffer.allocate(32);
				bb.order(ByteOrder.BIG_ENDIAN);
				bb.put(data, i, 4);
				bb.order(ByteOrder.LITTLE_ENDIAN);
				planeDataArray[i / 4] = bb.getInt(0);
			}

			// store data in new PlaneData object
			PlaneData pd = new PlaneData();
			pd.currLat = planeDataArray[0];
			pd.currLng = planeDataArray[1];
			pd.currAlt = planeDataArray[2];
			pd.nextLat = planeDataArray[3];
			pd.nextLng = planeDataArray[4];
			pd.nextAlt = planeDataArray[5];
			pd.ground_speed = planeDataArray[6];
			pd.target_bearing = planeDataArray[7];
			pd.currWP = planeDataArray[8];
			pd.WPdistance = planeDataArray[9];

			return pd;
		}
	}


}