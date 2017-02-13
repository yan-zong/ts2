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

import ts2.im.Nodo;
import ts2.im.Buffer3;
import ts2.im.Manager;

module RelayCore like IPtpCore
{
    parameters:

        @display("bgb=270,173,white");
    gates:
        input upperGateIn;	// from net layer
        output upperGateOut;	// to net layer
        input lowerGateIn;	// from mac, 
        output lowerGateOut;	// to mac

        output outclock;
        input inclock;

    submodules:
        RelaySlave: RelaySlave {
            parameters:
                @display("p=130,50;i=device/drive");
        }

        RelayMaster: RelayMaster {
            parameters:
                @display("p=130,120;i=device/drive");
        }

        RelayBuffer: RelayBuffer {
            parameters:
                @display("p=52,118;i=block/buffer");
        }
        manager: Manager {
            parameters:
                @display("p=201,53;i=block/cogwheel");
        }
    // move clock outside of ptpSlave module
    //        clock: Clock {
    //            parameters:
    //                @display("p=85,112;i=device/clock");
    //        }
    connections allowunconnected:
        RelaySlave.upperGateIn <-- { @display("ls=blue,1"); } <-- upperGateIn;
        RelaySlave.upperGateOut --> { @display("ls=blue,1"); } --> upperGateOut;

        // node.outclock --> clock.inclock;
        // node.inclock <-- clock.outclock;
        // ptpslave.outclock -->{ @display("ls=black,1"); }--> outclock;
        RelaySlave.inclock <-- { @display("ls=red,4"); } <-- inclock;

        RelaySlave.inevent <-- manager.out;

        RelaySlave.out --> RelayBuffer.in[0];
        RelayMaster.lowerGateOut --> RelayBuffer.in[1];
        
        RelayBuffer.out[0] --> RelaySlave.in;
        RelayBuffer.out[1] --> RelayMaster.lowerGateIn;
               
        RelayBuffer.in[2] <-- lowerGateIn;	
        RelayBuffer.out[2] --> lowerGateOut;	
}

