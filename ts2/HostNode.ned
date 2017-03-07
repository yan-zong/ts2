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

import org.mixim.modules.nic.IWirelessNicUpperCtrl;
import org.mixim.base.modules.IBaseArp;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;

module HostNode
{
    parameters:
        string networkType = default("BaseNetwLayer");	// type of the network layer
        string applicationType = default("BurstApplLayer");  // type of the application layer
        string mobilityType = default("StationaryMobility"); // type of the mobility module
        string arpType = default("ArpHost");
        string nicType;
        string ptpCoreType;
        @display("bgb=276,311,white;i=device/palm;b=40,40,rect");
        @node();

    gates:
        input radioIn; // gate for sendDirect
        
    submodules:
        arp: <arpType> like IBaseArp 
        {
            @display("p=212,184;i=block/network2");
        }
        
        mobility: <mobilityType> like inet.mobility.IMobility 
        {
             @display("p=212,37;i=block/cogwheel");
        }

        appl: <applicationType> like IBaseApplLayer 
        {
            @display("p=61,36;i=app;is=l");
        }

        netwl: <networkType> like IBaseNetwLayer 
        {
            @display("p=61,184;i=block/layer");
        }
        
        core: <ptpCoreType> like IPtpCore 
        {
            @display("p=127,102;i=block/layer");
        }

        nic: <nicType> like IWirelessNicUpperCtrl 
        {
            @display("p=61,253;i=block/wrxtx,#008000");
        }

    connections:
        nic.upperLayerOut --> netwl.lowerLayerIn;
        nic.upperLayerIn <-- netwl.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> netwl.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- netwl.lowerControlOut;

        netwl.upperLayerOut --> core.lowerGateIn;
        core.upperGateOut --> appl.lowerLayerIn;

        netwl.upperLayerIn <-- core.lowerGateOut;
        core.upperGateIn <-- appl.lowerLayerOut;

        netwl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        netwl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}
