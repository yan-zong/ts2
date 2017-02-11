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
import org.mixim.modules.phy.PhyLayer;
import mytdmamac.tdmamac.TDMAmac;

module Nic_Timestamp_TDMA like IWirelessNicUpperCtrl
{
    parameters:
        string connectionManagerName = default("");   //name of the ConnectionManager module
        string phyType = default("org.mixim.modules.phy.PhyLayer");
        string macType = default("TDMAmac");       
        
	gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect
	submodules:
        mac: TDMAmac {
            @display("p=74,46;i=block/layer;is=l");
        }

		timestamp: Timestamp {
            @display("p=29,101;b=,,oval,red;i=device/clock;is=s;tt=Time Stamp ");
        }

		phy: PhyLayer {
            @display("p=74,151;i=block/process_s;is=n");
        }
        
	connections:
        mac.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> upperLayerOut;
        mac.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- upperLayerIn;
        mac.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> upperControlOut;
        mac.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- upperControlIn;

		phy.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> timestamp.lowerGateIn;
        timestamp.upperGateOut-->{ @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerLayerIn;

        phy.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- timestamp.lowerGateOut;
        timestamp.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerLayerOut;

        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}










