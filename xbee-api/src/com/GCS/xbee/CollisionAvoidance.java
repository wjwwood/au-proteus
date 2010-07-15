package com.GCS.xbee;

import org.apache.log4j.Logger;

/**
 * Runs a collision avoidance algorithm as a thread for the XBeeGCS to run.
 * Asks the XBeeGCS to transmit a new waypoint to an ArduPilot in the air if necessary.
 * 
 * @author Varun Sampath <vsampath@seas.upenn.edu>
 * @author Andrew Proctor <procta06@highpoint.edu>
 */
class CollisionAvoidance implements Runnable {
	
	private XBeeGCS gcs;
	private static final Logger log = Logger.getLogger(CollisionAvoidance.class);	// logger for the log4j system
	
	public CollisionAvoidance(XBeeGCS gcs) {
		this.gcs = gcs;
	}

	/**
	 * Runs the collision avoidance algorithm after being notified of new data.
	 * 
	 * Note: this method should be thread-safe with regard to the XBeeGCS's internal hash map.
	 */
	@Override
	public void run() {
		synchronized (gcs.getDataMap()) {
			while (true) {		
				// wait for new data
				try {	gcs.getDataMap().wait();	} catch (InterruptedException e) { }

				// Start calculations after getting told from hash map about new data

				PlaneData plane = gcs.getDataMap().get(gcs.getLatest());		// the plane in question
				Coordinate sumRepulsive = new Coordinate();	// the sum of repulsive force vectors
				Coordinate unitAttract = new Coordinate();
				Coordinate unitRepulse = new Coordinate();
				Coordinate resultant = new Coordinate();
				Coordinate unitResultant = new Coordinate();
				
				// iterate over all other planes ("active objects") and calculate repulsive forces
				for (PlaneData active : gcs.getDataMap().values()) {
					if (active != plane) {
						Coordinate repulse = forceRepulsive(plane, active, unitRepulse);
						log.debug("The repulsive force vector is: " + repulse);
						sumRepulsive.x += repulse.x;
						sumRepulsive.y += repulse.y;
					}
				}

				// calculate an attractive force placed by the destination
				Coordinate attract = forceAttractive(plane, unitAttract);

				//Checking the special case of a target directly behind an obstacle
				//Checks unit vectors from forceRepulsive and forceAttractive functions
				if(unitRepulse.x + unitAttract.x == 0 && unitRepulse.y + unitAttract.y == 0){
					//Multiply the attract unit vector by the rotation matrix to place a small left hand turn into the path.
					unitResultant.x = (unitAttract.x * Math.cos(45)) - (unitAttract.y * Math.sin(45));
					unitResultant.y = (unitAttract.x * Math.sin(45)) + (unitAttract.y * Math.cos(45));
					log.debug("placing the left hand turn");
				} else { 
					resultant.x = sumRepulsive.x + attract.x;
					resultant.y = sumRepulsive.y + attract.y;
					log.debug("The resultant force vector is: " + resultant);

					//Calculates the unit vector of the resultant vector
					double rmag = Math.sqrt(Math.pow(resultant.x, 2) + Math.pow(resultant.y, 2));
					unitResultant.x = resultant.x/rmag;
					unitResultant.y = resultant.y/rmag;
				}
				log.debug("resultant unit vector: " + unitResultant);

				//Multiply a weight (step size) to the unit vector in direction or resultant vector
				final double weight = 1;
				resultant.x = weight * unitResultant.x;
				resultant.y = weight * unitResultant.y;

				// add the resultant force to the current waypoint to get the intermediate point
				Coordinate newWP = new Coordinate();
				newWP.x = resultant.x + plane.currLat;
				newWP.y = resultant.y + plane.currLng;
				newWP.z = plane.currAlt;

				gcs.transmit(gcs.getLatest(), newWP);
			}
		}
	}

	private double distance2D(double x1, double y1, double x0, double y0) {
		return Math.sqrt(Math.pow(x1-x0,2) + Math.pow(y1-y0,2));
	}

	private Coordinate forceRepulsive(PlaneData airplane, PlaneData active, Coordinate unitRepulse) {
		final double Fcr = 10;

		double distance = distance2D(active.currLat, active.currLng, airplane.currLat, airplane.currLng);

		double x = ((airplane.currLat - active.currLat) / distance);
		double y = ((airplane.currLng - active.currLng) / distance);
		
		unitRepulse.x = x;
		unitRepulse.y = y;

		log.debug("The repulsive force unit vector is <" + x + "," + y + ">");

		Coordinate repulse = new Coordinate();
		repulse.x = Fcr / Math.pow(distance, 2) * x;
		repulse.y = Fcr / Math.pow(distance, 2) * y;

		return repulse;
	}

	private Coordinate forceAttractive(PlaneData airplane, Coordinate unitAttract) {
		final double Fct = 5;

		double distance = distance2D(airplane.nextLat, airplane.nextLng, airplane.currLat, airplane.currLng);

		double x = ((airplane.nextLat - airplane.currLat) / distance);
		double y = ((airplane.nextLng - airplane.currLng) / distance);

		log.debug("The attractive force unit vector is: <" + x + "," + y + ">");
		
		unitAttract.x = x;
		unitAttract.y = y;

		Coordinate attract = new Coordinate();
		attract.x = Fct*x;
		attract.y = Fct*y;

		return attract;
	}


}
