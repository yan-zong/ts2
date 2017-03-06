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


module RelayNode extends HostCC2420a
{
    parameters:

        // two types of app-net layers
     //   applicationType = default("");  //type of the application layer
    //    networkType = "org.mixim.examples.ieee802154Narrow.NetworkStackTrafficGen";

        applicationType = default("BurstApplLayerBattery");  //type of the application layer
        networkType = "BaseNetwLayer";

        mobilityType = default("StationaryMobility"); //type of the mobility module
        arpType = "ArpHost";


		// ####	NIC (CSMA, TDMA), and Hardware TimeStamp ####
		// for NIC without hardware time stamp, and also for CSMA
		// nicType = "Nic802154_TI_CC2420";
      
		// for NIC with hardware timie stamp, and also for CSMA
		// nicType = "Nic_CC2420_TmStmp";
		// nic.tsstmp.useGlobalRefClock = default(false);
        // set useGlobalRefClock = true to use simTime() for time stamping
        // set useGlobalRefClock = false to use clock module for time stamping if a node has a clock module
		
		// for NIC with hardware timie stamp, and also for TDMA
		nicType = "Nic_Timestamp_TDMA";

        // ####	End of NIC (CSMA, TDMA), Hardware TimeStamp ####

        ptpCoreType = default("RelayCore");
        int numHosts; // total number of hosts in the network

        @display("bgb=271,307,white;i=misc/node;i2=status/battery;b=40,40,rect;is=s");

    submodules:
        batteryStats: BatteryStats {
            @display("p=215,253;i=block/table,#FF8040");
        }
        battery: SimpleBattery {
            @display("p=133,253;i=block/plug,#FF8000");
        }
        // move clock outside of ptpSlave module
        clock: PCOClock {
            parameters:
                @display("p=215,101;i=device/clock");
        }

    connections allowunconnected:

        ptpcore.outclock --> clock.inclock;
        ptpcore.inclock <-- clock.outclock;


}
