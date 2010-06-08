/**
 * Copyright (c) 2008 Andrew Rapp. All rights reserved.
 *  
 * This file is part of XBee-API.
 *  
 * XBee-API is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * XBee-API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with XBee-API.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.rapplogic.xbee.examples.zigbee;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.api.zigbee.ZNetTxStatusResponse;
import com.rapplogic.xbee.util.ByteUtils;

/** 
 * @author andrew
 */
public class ArduinoZBTxTest {

	private final static Logger log = Logger.getLogger(ArduinoZBTxTest.class);
	
	private int pwm = 0;
	
	private ArduinoZBTxTest() throws XBeeException {
		
		XBee xbee = new XBee();
		
		try {
			// replace with your com port and baud rate. this is the com port of my coordinator
			//xbee.open("COM5", 9600);
			// my coordinator com/baud
			xbee.open("/dev/tty.usbserial-A6005v5M", 9600);
			// my end device
			//xbee.open("/dev/tty.usbserial-A6005uPi", 9600);
			
			// replace with end device's 64-bit address (SH + SL)
			XBeeAddress64 addr64 = new XBeeAddress64(0, 0x13, 0xa2, 0, 0x40, 0x0a, 0x3e, 0x02);
			
		
			// create an array for pwm value
			int[] weather = ByteUtils.stringToIntArray("Current Conditions: Fair, 73 F");
			
			// first request we just send 64-bit address.  we get 16-bit network address with status response
			ZNetTxRequest request = new ZNetTxRequest(addr64, weather);
			
			log.debug("zb request is " + request.getXBeePacket().getPacket());
			
			log.info("sending tx " + request);
			
			while (true) {
				long start = System.currentTimeMillis();
				//log.info("sending tx packet: " + request.toString());
				
				try {
					ZNetTxStatusResponse response = (ZNetTxStatusResponse) xbee.sendSynchronous(request, 10000);
					// update frame id for next request
					request.setFrameId(xbee.getNextFrameId());
					
					log.info("received response " + response.toString());
					
					log.debug("status response is " + ByteUtils.toBase16(response.getPacketBytes()));

					if (response.getDeliveryStatus() == ZNetTxStatusResponse.DeliveryStatus.SUCCESS) {
						// the packet was successfully delivered
						if (response.getRemoteAddress16().equals(XBeeAddress16.ZNET_BROADCAST)) {
							// specify 16-bit address for faster routing?.. really only need to do this when it changes
							request.setDestAddr16(response.getRemoteAddress16());
						}							
					} else {
						// packet failed.  log error
						// it's easy to create this error by unplugging/powering off your remote xbee.  when doing so I get: packet failed due to error: ADDRESS_NOT_FOUND  
						log.error("packet failed due to error: " + response.getDeliveryStatus());
					}
					
					// I get the following message: Response in 75, Delivery status is SUCCESS, 16-bit address is 0x08 0xe5, retry count is 0, discovery status is SUCCESS 
					log.info("Response in " + (System.currentTimeMillis() - start) + ", Delivery status is " + response.getDeliveryStatus() + ", 16-bit address is " + ByteUtils.toBase16(response.getRemoteAddress16().getAddress()) + ", retry count is " +  response.getRetryCount() + ", discovery status is " + response.getDeliveryStatus());					
				} catch (XBeeTimeoutException e) {
					log.warn("request timed out");
				}
	
				try {
					// wait a bit then send another packet
					Thread.sleep(60000);
				} catch (InterruptedException e) {
				}
				
				pwm+=25;
//				
//				if (pwm > 255) {
//					pwm = 0;
//				}
//				
//				pwmData[0] = pwm;
			}

		} finally {
			xbee.close();
		}
	}
	
	public static void main(String[] args) throws XBeeException, InterruptedException  {
		PropertyConfigurator.configure("log4j.properties");
		new ArduinoZBTxTest();
	}
}
