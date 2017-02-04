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
import ts2.im.Clock;

module SlaveCore like ICore
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
        slave: Slave {
            parameters:
                @display("p=130,85;i=device/drive");
        }

        buffertx: Buffer3 {
            parameters:
                @display("p=52,118;i=block/buffer");
        }
        bufferrx: Buffer3 {
            parameters:
                @display("p=52,53;i=block/buffer");
        }
        manager: Manager {
            parameters:
                @display("p=201,53;i=block/cogwheel");
        }
        // clock: Clock {
    	// 	parameters:
        //         @display("p=85,112;i=device/clock");
        // }
        
        
    connections allowunconnected:
        slave.upperGateIn <-- { @display("ls=blue,1"); } <-- upperGateIn;
        slave.upperGateOut --> { @display("ls=blue,1"); } --> upperGateOut;

        slave.inclock <-- { @display("ls=red,4"); } <-- inclock;

        slave.inevent <-- manager.out;

        slave.out --> buffertx.in;
        slave.in <-- bufferrx.out;

        buffertx.out --> lowerGateOut;
        bufferrx.in <-- lowerGateIn;


}

