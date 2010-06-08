package com.rapplogic.xbee.examples.zigbee;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.util.ByteUtils;

public class ArduinoPackets {

	private final static Logger log = Logger.getLogger(ArduinoPackets.class);
	
	public ArduinoPackets() {		
		// my coordinator
		XBeeAddress64 addr64 = new XBeeAddress64(0x00, 0x13, 0xa2, 0x00, 0x40, 0x3e, 0x0f, 0x30);
		
		// create an array of arbitrary data to send
		int[] payload = new int[] { 'H', 'i' };
		
		// first request we just send 64-bit address.  we get 16-bit network address with status response
		ZNetTxRequest request = new ZNetTxRequest(addr64, payload);
		request.setFrameId(0);
		
		log.debug("zb request is " + ByteUtils.toBase16(request.getXBeePacket().getPacket()));
		
	}
	
	public static void main(String[] args) {
		PropertyConfigurator.configure("log4j.properties");
		new ArduinoPackets();
	}
}
