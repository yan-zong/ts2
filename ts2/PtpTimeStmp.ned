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

//
// TODO auto-generated module
//
simple PtpTimeStmp
{
       parameters:
        @class(PtpTimeStmp);
        bool useGlobalRefClock=default(true); 
           // true, accurate clock that uses the simTime() as the clock source (,
           // false, inaccurate clock that uses the clock module in the node as the clock source
           
    gates:
        input upperGateIn; // from mac layer
        output upperGateOut; // to mac layer
        input lowerGateIn; // from phy
        output lowerGateOut; // to phy
    
}
