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

simple MasterCore  like IPtpCore
{
    parameters:
       @class(PtpMaster);
    	double Tsync = default(1);
        int masterAddrOffset;
            
    gates:
        input upperGateIn; // from net layer
        output upperGateOut; // to net layer
    //    input upperControlIn; // control from net layer
    //    output upperControlOut; // control to net layer
        input lowerGateIn; // from mac
        output lowerGateOut; // to mac
    //    input lowerControlIn; // control from mac
    //    output lowerControlOut; // control to mac
        
        output outclock;
        input inclock;
        
}   
