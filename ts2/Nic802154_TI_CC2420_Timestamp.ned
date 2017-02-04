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

import org.mixim.modules.nic.WirelessNicBattery;
import org.mixim.modules.nic.WirelessNic;

import org.mixim.base.phyLayer.IWirelessPhy;
import org.mixim.base.modules.IWirelessMac;
import org.mixim.modules.nic.IWirelessNicUpperCtrl;


// ****************************************************************************
// This NIC implements a Texas Instruments CC 2420 802.15.4 network interface 
// card using the CSMA protocol as specified in IEEE 802.15.4-2006.
// The additional module Timestamp.ned is for set the hardware time stamp to 
// PtpPkt.msg.
//
// Note: To be able to use this NIC in your simulation you have to copy and  
// keep the file "Nic802154_TI_CC2420_Decider.xml" at the same directory as 
// NIC.ned
// ****************************************************************************

module Nic802154_TI_CC2420_Timestamp like IWirelessNicUpperCtrl
{
    parameters:
        string connectionManagerName = default("");   //name of the ConnectionManager module, used for multi channel connection
        // string phyType = default("org.mixim.modules.phy.PhyLayer");
        string phyType = default("org.mixim.modules.phy.PhyLayerBattery");

        // string macType = default("org.mixim.modules.mac.CSMAMacLayer");
        string macType = default("CSMA802154");

        double sleepCurrent @unit(mA);
        double rxCurrent @unit(mA);
        double decodingCurrentDelta @unit(mA);
        double txCurrent @unit(mA);
        double setupRxCurrent @unit(mA);
        double setupTxCurrent @unit(mA);
        double rxTxCurrent @unit(mA);
        double txRxCurrent @unit(mA);

        // power consumption from boards (at 3.3V)
        sleepCurrent = 0.000021mA; // 0.021 µA
        rxCurrent = 18.8 mA;
        decodingCurrentDelta = 0 mA;
        txCurrent = 17.4 mA;
        setupRxCurrent = 0.6391mA; // 0.002109 W = (ESetupPower+ESetupXtal+ETxCalibrate)/TSetupTx
        setupTxCurrent = 0.6845mA; // 0.002259 W = (ESetupPower+ESetupXtal+ERxCalibrate)/TSetupRx
        rxTxCurrent = 18.8 mA; // Upper bound
        txRxCurrent = 18.8 mA; // idem

        phy.decider = xmldoc("Nic802154_TI_CC2420_Decider.xml");
        //publishRSSIAlways = false;
        phy.headerLength = 48 bit; // ieee 802.15.4                
        phy.thermalNoise = default(-110 dBm);
        // From TI CC1100 datasheet rev. C
        phy.timeSleepToRX = 0.001792 s;
        phy.timeSleepToTX = 0.001792 s;
        phy.timeRXToTX = 0.000192 s;
        phy.timeTXToRX = 0.000192 s;
        phy.timeRXToSleep = 0 s;
        phy.timeTXToSleep = 0 s;

        mac.rxSetupTime = 0.001792 s;
        mac.txPower = default(1 mW);

        @display("bgb=135,186,white");
    gates:
        input upperLayerIn; // to upper layers
        output upperLayerOut; // from upper layers
        output upperControlOut; // control information 
        input upperControlIn; // control information 
        input radioIn; // radioIn gate for sendDirect

    submodules:
        mac: <macType> like IWirelessMac {
            @display("p=74,46;i=block/layer;is=l");
        }
        timestamp: Timestamp {

            @display("p=29,101;b=,,oval,red;i=device/clock;is=s;tt=Time Stamp ");
        }

        phy: <phyType> like IWirelessPhy {
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

