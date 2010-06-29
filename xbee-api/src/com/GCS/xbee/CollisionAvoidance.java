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
	private int planeCounter;
	
	public CollisionAvoidance(XBee xbee, Logger log) {
		CollisionAvoidance.xbee = xbee;
		CollisionAvoidance.log = log;
		dataMap = new HashMap<XBeeAddress64, PlaneData>();
		planeCounter = 0;
		new Thread(new Avoid()).start();
	}

	// update hash map data and inform collision avoidance thread
	public void addData(XBeeAddress64 addr, PlaneData data) {
		if (data != null) {
			// associate a plane number with the incoming data
			if (!dataMap.containsKey(addr))
				data.num = ++planeCounter;
			else 
				data.num = dataMap.get(addr).num;
			// update hash map
			latest = addr;
			log.info(data);
			synchronized (dataMap) {
				dataMap.put(addr, data);
				dataMap.notify();
			}
		}
	}
	
	// Assume data has 3 fields: latitude, longitude, and altitude
	private void transmit(XBeeAddress64 addr, int[] waypoint) {
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
		// undo the multipliers to make logging prettier
		waypoint[0] /= 10; waypoint[1] /= 10; waypoint[2] /= 100;
		log.info("sent " + Arrays.toString(waypoint) + " to Plane " + dataMap.get(addr).num);
	}

	private class Coordinate {
		double x;
		double y;
		double z;
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
					Coordinate sumRepulsive = new Coordinate();
					sumRepulsive.x = 0; sumRepulsive.y = 0; sumRepulsive.z = 0;

					for (PlaneData airplane : dataMap.values()) {
						if (airplane != dataMap.get(latest)) {
							Coordinate repulse = new Coordinate();
							forceRepulsive(dataMap.get(latest), airplane, repulse);
							log.debug("The repulsive force vector is: <" + repulse.x + "," + repulse.y + ">");
							sumRepulsive.x += repulsive.x;
							sumRepulsive.y += repulsive.y;
						}
					}

					Coordinate attract = new Coordinate();
					forceAttractive(latest, attract);

					Coordinate resultant = new Coordinate();
					resultant.x = sumRepulsive.x + attract.x;
					resultant.y = sumRepulsive.y + attract.y;

					log.debug("The resultant force vector is: <" + resultant.x + "," + resultant.y + ">");

					Coordinate newWP = new Coordinate();
					newWP.x = resultant.x + dataMap.get(latest).currLat;
					newWP.y = resultant.y + dataMap.get(latest).currLng;
					newWP.z = dataMap.get(latest).currAlt;

					int[] waypoint = {32594727,-85497500,250};		// south intramural field, HIGH alt
//					transmit(addr, waypoint);
				}
			}
		}

		private double distance2D(double x1, double y1, double x0, double y0) {
			return Math.sqrt(Math.pow(x1-x0,2) + Math.pow(y1-y0,2));
		}

		private void forceRepulsive(PlaneData airplane, PlaneData active, Coordinate repulse) {
			double distance;
			double x, y;
			int[] active = new int[3];
			final double Fcr = 10;

			distance = distance2D(active.currLat, active.currLng, airplane.currLat, airplane.currLng);

			x = ((airplane.currLat - active.currLat) / distance);
			y = ((airplane.currLng - active.currLng) / distance);

			log.debug("The repulsive force unit vector is <" + x + "," + y + ">");
			repulse.x = (Fcr / Math.pow(distance, 2)) * ((airplane.x - active.x) / distance);
			repulse.y = (Fcr / Math.pow(distance, 2)) * ((airplane.y - active.y) / distance);
		}

		private void forceAttractive(PlaneData airplane, Coordinate attract) {
			double distance;
			double x, y, x2, y2;
			final double Fct = 5;

			distance = distance2D(airplane.nextLat, airplane.nextLng, airplane.currLat, airplane.currLng);

			x = ((airplane.nextLat - airplane.currLat) / distance);
			y = ((airplane.nextLng - airplane.currLng) / distance);

			log.debug("The attractive force unit vector is: <" + x + "," + y + ">");

			attract.x = Fct*(airplane.nextLat - airplane.currLat) / distance;
			attract.y = Fct*(airplane.nextLng - airplane.currLng) / distance;
		}

		
	}

}
