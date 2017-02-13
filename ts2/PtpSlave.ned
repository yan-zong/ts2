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

import ts2.im.Nodo;
import ts2.im.Buffer3;
import ts2.im.Manager;
import ts2.im.Clock;

module PtpSlave like IPtpCore
{
    parameters:

        @display("bgb=270,173,white");
    gates:
//        input in; // replaced by lowerGateIn
//        output out; // replaced by lowerGateOut

        input upperGateIn; // from net layer
        output upperGateOut; // to net layer
        input lowerGateIn; // from mac, 
        output lowerGateOut; // to mac

        output outclock;
        input inclock;

    submodules:
        ptpslave: Node {
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
    // move clock outside of ptpSlave module
    //        clock: Clock {
    //            parameters:
    //                @display("p=85,112;i=device/clock");
    //        }
    connections allowunconnected:
        ptpslave.upperGateIn <-- { @display("ls=blue,1"); } <-- upperGateIn;
        ptpslave.upperGateOut --> { @display("ls=blue,1"); } --> upperGateOut;

        // node.outclock --> clock.inclock;
        // node.inclock <-- clock.outclock;
        // ptpslave.outclock -->{ @display("ls=black,1"); }--> outclock;
        ptpslave.inclock <-- { @display("ls=red,4"); } <-- inclock;

        ptpslave.inevent <-- manager.out;

        ptpslave.out --> buffertx.in;
        ptpslave.in <-- bufferrx.out;

        buffertx.out --> lowerGateOut;
        bufferrx.in <-- lowerGateIn;


}

