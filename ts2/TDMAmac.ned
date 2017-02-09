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


// TDMAmac --> BaseMacLayer --> BaseLayer --> IWirelessMac
// CSMA802154 --> csma --> BaseMacLayer --> BaseLayer --> IWirelessMac
package ts2;

import org.mixim.base.modules.BaseMacLayer;

simple TDMAmac extends BaseMacLayer
{
    parameters:
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

    	@class("TDMAmac");
}







