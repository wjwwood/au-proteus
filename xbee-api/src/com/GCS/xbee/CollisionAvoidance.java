package com.GCS.xbee;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
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
	private Thread avoid;
	private XBeeAddress64 latest;
	
	public CollisionAvoidance(XBee xbee, Logger log) {
		CollisionAvoidance.xbee = xbee;
		CollisionAvoidance.log = log;
		dataMap = new HashMap<XBeeAddress64, PlaneData>();
		avoid = new Thread(new Avoid());
		avoid.start();
	}

	public void addData(XBeeAddress64 addr, PlaneData data) {
		if (data != null) {
		synchronized (this) {
			dataMap.remove(addr);
			dataMap.put(addr, data);
			latest = addr;
		}
		}
		
		latest = addr;
		avoid.interrupt();
	}
	
	// Assume data has 3 fields: latitude, longitude, and altitude
	private void transmit(XBeeAddress64 addr, int[] waypoint) {
		if (waypoint.length != 3) throw new IllegalArgumentException();
		
		int payload[] = new int[16];	//4 int32's
		
		for (int i = 0; i < waypoint.length; i++) {
			ByteBuffer bb = ByteBuffer.allocate(32);
			bb.order(ByteOrder.LITTLE_ENDIAN);
			bb.putInt(waypoint[i]);
			bb.order(ByteOrder.BIG_ENDIAN);
			for (int j = 0; j < 4; j++) {
				payload[4*i+j] = bb.array()[j];
			}
		}
		
		// for the checksum
		ByteBuffer bb = ByteBuffer.allocate(32);
		bb.order(ByteOrder.LITTLE_ENDIAN);
		bb.putInt(waypoint[0] + waypoint[2]);
		bb.order(ByteOrder.BIG_ENDIAN);
		for (int j = 0; j < 4; j++) {
			payload[12+j] = bb.array()[j];
		}
		
		ZNetTxRequest request = new ZNetTxRequest(addr, payload);
		log.debug("zb request is " + request.getXBeePacket().getPacket());	
		//log.info("sending tx " + request);
		//log.info("request is " + ByteUtils.toBase10(request.getXBeePacket().getPacket()));
		try {
			xbee.sendAsynchronous(request);
		} catch (XBeeException e) {
			e.printStackTrace();
		}
	}
	
	private class Avoid implements Runnable {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			while (true) {
				try {	Thread.sleep(Long.MAX_VALUE);	}
				catch (InterruptedException e) { }
				
				// Do magic in here?
				XBeeAddress64 addr = latest;
				int[] waypoint = {32605800, -85487900, 300};
				//int[] waypoint = {0,0,0};
				//int[] waypoint = {123,456,789};
				transmit(addr, waypoint);
			}
		}
		
	}

}
