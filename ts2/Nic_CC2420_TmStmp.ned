//***************************************************************************
// * file:        Nic_CC2420_TmStmp.ned
// *
// * author:      Xuweu Dai
// *
// * copyright:   (C) 2014 Southwest University, Chongqing, China.
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// *
// * Credit:      Yiwen Huang, Taihua Li
// * Funding: This work was partially financed by the National Science Foundation China under the  
// * grant ""  
// **************************************************************************/

package ts2;

import org.mixim.modules.nic.WirelessNicBattery;
import org.mixim.modules.nic.WirelessNic;

import org.mixim.base.phyLayer.IWirelessPhy;
import org.mixim.base.modules.IWirelessMac;
import org.mixim.modules.nic.IWirelessNicUpperCtrl;


//
// This NIC implements a Texas Instruments CC 2420 802.15.4 network interface card
// using the CSMA protocol as specified in IEEE 802.15.4-2006.
// The additional module PtpTimeStmp.ned is for set the hardware time stamp to PtpPkt.msg.
//
//Note: To be able to use this Nic in your simulation you have to copy and keep the file 
//"Nic802154_TI_CC2420_Decider.xml" at the same directory as Nic_CC2420_TmStmp.ned
// @author Xuewu Dai
//

module Nic_CC2420_TmStmp like IWirelessNicUpperCtrl
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
        tsstmp: TimeStamp {

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


        phy.upperLayerOut --> { @display("ls=black;m=m,25,50,25,0"); } --> tsstmp.lowerGateIn;
        tsstmp.upperGateOut-->{ @display("ls=black;m=m,25,50,25,0"); } --> mac.lowerLayerIn;

        phy.upperLayerIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- tsstmp.lowerGateOut;
        tsstmp.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- mac.lowerLayerOut;

        phy.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> mac.lowerControlIn;
        phy.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- mac.lowerControlOut;

        radioIn --> phy.radioIn;

}

