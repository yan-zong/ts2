//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  29 Jan 2014
// * Author:      Xuweu Dai  (x.dai at ieee.org)
// *
// * Copyright:   (C) 2014 Southwest University, Chongqing, China.
// *
// *              TS2 is free software; you can redistribute it  and/or modify
// *              it under the terms of the GNU General Public License as published
// *              by the Free Software Foundation; either  either version 3 of
// *              the License, or (at your option) any later version.
// *
// *              TS2 is distributed in the hope that it will be useful,
// *                  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *                  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *                  GNU General Public License for more details.
// *
// * Credit:      Yiwen Huang, Taihua Li
// * Funding:     This work was partially financed by the National Science Foundation China
// %              _
// %  \/\ /\ /   /  * _  '
// % _/\ \/\/ __/__.'(_|_|_
// **************************************************************************/


package ts2;

import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.node.WirelessNode;


module MasterNode extends HostCC2420a
{
    parameters:

        // two types of app-net layers
     //   applicationType = default("");  //type of the application layer
    //    networkType = "org.mixim.examples.ieee802154Narrow.NetworkStackTrafficGen";

        applicationType = default("BurstApplLayerBattery");  //type of the application layer
        networkType = "BaseNetwLayer";

        mobilityType = default("StationaryMobility"); //type of the mobility module
        arpType = "ArpHost";


		// #####   nic and hardware time stamp      #####
		// for NIC without hardware time stamp
      nicType = "Nic802154_TI_CC2420";

        // for NIC with hardware timie stamp
//        nicType = "Nic_CC2420_TmStmp";
//        nic.tsstmp.useGlobalRefClock = default(true);
         // set useGlobalRefClock = true to use simTime() for time stamping
         // set useGlobalRefClock = false to use clock module for time stamping if a node has a clock module

        // ##### end of nic and hardware time stamp #####

        CoreType = default("MasterCore");
        int numHosts; // total number of hosts in the network

        @display("bgb=273,307,white;i=misc/node;i2=status/battery;b=40,40,rect;is=s");

    submodules:
        batteryStats: BatteryStats {
            @display("p=212,253;i=block/table,#FF8040");
        }
        battery: SimpleBattery {
            @display("p=135,253;i=block/plug,#FF8000");
        }
        
        clock: Clock2 {
            parameters:
                @display("p=215,101;i=device/clock");
        }
    connections allowunconnected:
        core.outclock --> clock.inclock;
        core.inclock <-- clock.outclock;
        
}
