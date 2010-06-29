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
	private void transmit(XBeeAddress64 addr, Coordinate wp) {
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
					// wait for new data
					try {	dataMap.wait();	} catch (InterruptedException e) { }
					
					// Start calculations after getting told from hash map about new data
					
					PlaneData plane = dataMap.get(latest);		// the plane in question
					Coordinate sumRepulsive = new Coordinate();	// the sum of repulsive force vectors
					sumRepulsive.x = 0; sumRepulsive.y = 0; sumRepulsive.z = 0;

					// iterate over all other planes ("active objects") and calculate repulsive forces
					for (PlaneData active : dataMap.values()) {
						if (active != plane) {
							Coordinate repulse = forceRepulsive(plane, active);
							log.debug("The repulsive force vector is: <" + repulse.x + "," + repulse.y + ">");
							sumRepulsive.x += repulse.x;
							sumRepulsive.y += repulse.y;
						}
					}

					// calculate an attractive force placed by the destination
					Coordinate attract = forceAttractive(plane);

					// sum the repulsive and attractive forces
					Coordinate resultant = new Coordinate();
					resultant.x = sumRepulsive.x + attract.x;
					resultant.y = sumRepulsive.y + attract.y;

					log.debug("The resultant force vector is: <" + resultant.x + "," + resultant.y + ">");

					// add the resultant force to the current waypoint to get the intermediate point
					Coordinate newWP = new Coordinate();
					newWP.x = resultant.x + plane.currLat;
					newWP.y = resultant.y + plane.currLng;
					newWP.z = plane.currAlt;

					//int[] waypoint = {32594727,-85497500,250};		// south intramural field, HIGH alt
					transmit(latest, newWP);
				}
			}
		}

		private double distance2D(double x1, double y1, double x0, double y0) {
			return Math.sqrt(Math.pow(x1-x0,2) + Math.pow(y1-y0,2));
		}

		private Coordinate forceRepulsive(PlaneData airplane, PlaneData active) {
			final double Fcr = 10;

			double distance = distance2D(active.currLat, active.currLng, airplane.currLat, airplane.currLng);

			double x = ((airplane.currLat - active.currLat) / distance);
			double y = ((airplane.currLng - active.currLng) / distance);

			log.debug("The repulsive force unit vector is <" + x + "," + y + ">");
			
			Coordinate repulse = new Coordinate();
			repulse.x = Fcr / Math.pow(distance, 2) * x;
			repulse.y = Fcr / Math.pow(distance, 2) * y;
			
			return repulse;
		}

		private Coordinate forceAttractive(PlaneData airplane) {
			final double Fct = 5;

			double distance = distance2D(airplane.nextLat, airplane.nextLng, airplane.currLat, airplane.currLng);

			double x = ((airplane.nextLat - airplane.currLat) / distance);
			double y = ((airplane.nextLng - airplane.currLng) / distance);

			log.debug("The attractive force unit vector is: <" + x + "," + y + ">");

			Coordinate attract = new Coordinate();
			attract.x = Fct*x;
			attract.y = Fct*y;
			
			return attract;
		}

		
	}

}
