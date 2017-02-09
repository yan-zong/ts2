//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Dov 2016
// * Author:      Yan Zong, Xuweu Dai
// *
// * Copyright:   (C) 2016 Northumbria University, UK.
// *
// *              TS2 is free software; you can redistribute it and/or modify it
// *              under the terms of the GNU General Public License as published
// *              by the Free Software Foundation; either version 3 of the
// *              License, or (at your option) any later version.
// *
// *              TS2 is distributed in the hope that it will be useful, but
// *              WITHOUT ANY WARRANTY; without even the implied warranty of
// *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *              GNU General Public License for more details.
// *
// * Funding:     This work was financed by the Northumbria University Faculty
//                Funded and RDF funded studentship, UK
// ****************************************************************************

package ts2;

// package org.mixim.base.modules;

// 'myTDMAmac(TDMAmac)' extend 'BaseMacLayer' extend 'BaseLayer' like 'IWirelessMac'
simple TDMAmac
{
    
    parameters:
    	// @class("myTDMAmac");
    	@class(TDMAmac);
    	
    	// enable per module debug
        bool debug = default(false);
        // how long is a slot?
        double slotDuration @unit(s) = default(100ms);
        // length of MAC header
        double queueLength = default(10);
        // default channel 
        double defaultChannel = default(0);
        // bit rate 
        double bitrate @unit(bps) = default(19200bps);
        // level at which we consider medium busy FIXME not used currently
		// double busyRSSI;
		
		bool stats = default(false);
		bool trace = default(false);
		
        double txPower = default(50);
	
    	// from BaseMacLayer.ned
    	// @class(BaseMacLayer);
    	bool coreDebug = default(false);          // debug switch
        int    headerLength @unit(bit);           // length of the MAC packet header (in bits)
        string address = default("auto");         // MAC address as hex string (12 hex digits), or
                                                  // "auto". "auto" values will be replaced by
                                                  // a generated MAC address in init stage 1.
    	
    	// from BaseLayer.ned
    	bool notAffectedByHostState = default(false);
    	
    	// from BaseLayer.ned (or IWirelessMac.ned)
    gates:
        input upperLayerIn; // from upper layer
        output upperLayerOut; // to uppe layer
        input upperControlIn; // control from upper layer
        output upperControlOut; // control to upper layer
        input lowerLayerIn; // from lower layer
        output lowerLayerOut; // to lower layer
        input lowerControlIn; // control from lower layer
        output lowerControlOut; // control to lower layer

}







