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

import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.node.WirelessNode;
import mytdmamac.TDMAmac.PCOClock;

module MasterCC2420 extends HostCC2420a
{
    parameters:
        applicationType = default("BurstApplLayerBattery");	// type of the application layer
        networkType = "BaseNetwLayer";

        mobilityType = default("StationaryMobility");	// type of the mobility module
        arpType = "ArpHost";

      	// nicType = "Nic802154_TI_CC2420";	// for NIC without hardware time stamp, and also for CSMA
 		nicType = "Nic_CC2420_TmStmp";	// for NIC with hardware timie stamp, and also for CSMA
		
        ptpCoreType = default("MasterCore");
        
        @display("bgb=273,307,white;i=misc/node;i2=status/battery;b=40,40,rect;is=s");

		int numHosts; // total number of hosts in the network
		
    submodules:
        batteryStats: BatteryStats 
        {
            @display("p=212,253;i=block/table,#FF8040");
        }
        
        battery: SimpleBattery 
        {
            @display("p=135,253;i=block/plug,#FF8000");
        }
        
        clock: PCOClock 
        {
             @display("p=215,101;i=device/clock");
        }
        
    connections allowunconnected:
        core.outclock --> clock.inclock;
        core.inclock <-- clock.outclock;
        
}
