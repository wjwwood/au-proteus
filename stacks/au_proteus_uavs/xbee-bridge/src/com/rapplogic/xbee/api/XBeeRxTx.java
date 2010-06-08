package com.rapplogic.xbee.api;

import java.io.IOException;

import org.apache.log4j.Logger;

import com.rapplogic.xbee.RxTxSerialComm;

public class XBeeRxTx extends RxTxSerialComm {
	
	private final static Logger log = Logger.getLogger(XBeeRxTx.class);
	
	private XBeePacketParser parser;
	
	public XBeeRxTx() {
		super();
		//parser = new XBeePacketParser(rxtx.getInputStream(), this, newPacketNotification);			
	}
	
	/**
	 * Called by RXTX to notify us that data is available to be read.
	 * 
	 * This method calls notify on the parser to indicate it should start
	 * consuming packets.
	 * 
	 */
	@Override
	protected void handleSerialData() throws IOException {
		log.info("RXTX serialEvent");

		// alert the parser we have new data
		// parser may not be waiting
		synchronized (parser) {
			parser.notify();
		}		
	}
}
