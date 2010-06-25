package com.GCS.xbee;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.util.HashMap;

import org.apache.log4j.Logger;

import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;

public class CollisionAvoidance {

	private static XBee xbee;
	private static Logger log;
	private HashMap<XBeeAddress64, PlaneData> dataMap;
	private XBeeAddress64 latest;
	
	public CollisionAvoidance(XBee xbee, Logger log) {
		CollisionAvoidance.xbee = xbee;
		CollisionAvoidance.log = log;
		dataMap = new HashMap<XBeeAddress64, PlaneData>();
		new Thread(new Avoid()).start();
	}

	// update hash map data and inform collision avoidance thread
	public void addData(XBeeAddress64 addr, PlaneData data) {
		if (data != null) {
			synchronized (dataMap) {
				dataMap.remove(addr);
				dataMap.put(addr, data);
				latest = addr;
			}
			dataMap.notify();
		}
	}
	
	// Assume data has 3 fields: latitude, longitude, and altitude
	private void transmit(XBeeAddress64 addr, int[] waypoint) {
		if (waypoint.length != 3) throw new IllegalArgumentException();
		
		int payload[] = new int[16];	//4 int32's
		
		// ArduPilot asks for these multipliers
		waypoint[0] *= 10; waypoint[1] *= 10; waypoint[2] *= 100;
		
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
			xbee.sendAsynchronous(request);
		} catch (XBeeException e) {
			e.printStackTrace();
		}
	
		// log output
		log.debug("zb request is " + request.getXBeePacket().getPacket());
		log.info("sent " + Arrays.toString(waypoint) + " to " + addr);
	}
	
	// thread that runs collision avoidance algorithm
	private class Avoid implements Runnable {
		@Override
		public void run() {
			synchronized (dataMap) {
				while (true) {		
					try {
						dataMap.wait();
					} catch (InterruptedException e) { }
					
					// Do algorithm calculations in here after getting told from hash map about new data
					XBeeAddress64 addr = latest;
					int[] waypoint = {32594727,-85497500,250};		// south intramural field, HIGH alt
					transmit(addr, waypoint);
				}
			}
		}
		
	}

}
