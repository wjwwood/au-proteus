package com.GCS.xbee_test;

import java.io.File;
import java.io.IOException;

import com.rapplogic.xbee.api.ApiId;
import com.rapplogic.xbee.api.AtCommand;
import com.rapplogic.xbee.api.AtCommandResponse;
import com.rapplogic.xbee.api.XBee;
import com.rapplogic.xbee.api.XBeeAddress16;
import com.rapplogic.xbee.api.XBeeAddress64;
import com.rapplogic.xbee.api.XBeeException;
import com.rapplogic.xbee.api.XBeeResponse;
import com.rapplogic.xbee.api.XBeeTimeoutException;
import com.rapplogic.xbee.api.zigbee.ZNetTxRequest;
import com.rapplogic.xbee.api.zigbee.ZNetTxStatusResponse;

public class Shared {

	static int getRSSI(XBee xbee) {
		try {
			AtCommandResponse response = (AtCommandResponse) xbee.sendSynchronous(new AtCommand("DB"), 5000);
			if (response.isOk()) {
				return -1*response.getValue()[0];
			}
		} catch (XBeeTimeoutException e) {
			e.printStackTrace();
		} catch (XBeeException e) {
			e.printStackTrace();
		}
		return 1;
	}
	
	static void createFile(String filename) throws IOException {
		File f = new File(filename);
		if (f.exists()) {
			f.delete();
		}
		if (!f.createNewFile()) throw new IOException("Could not create file");
	}
	
	static XBeeAddress16 get16Addr(XBee xbee, XBeeAddress64 dest_64) {
		XBeeResponse resp = null;
		try {
			resp = xbee.sendSynchronous(new ZNetTxRequest(dest_64, new int[]{1}), 5000);
		} catch (XBeeTimeoutException e) {
			e.printStackTrace();
		} catch (XBeeException e) {
			e.printStackTrace();
		}
		if (resp.getApiId() == ApiId.ZNET_TX_STATUS_RESPONSE)
			return ((ZNetTxStatusResponse) resp).getRemoteAddress16();
		return null;
	}
}
