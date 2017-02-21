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

simple RelaySlave
{
    parameters:
       @class(RelaySlave);
      //volatile double Tsync;
      //  volatile double nsync;
      volatile double Tcamp;
      volatile double rate;
    //  volatile double alpha;
      //volatile double  beta;
      //volatile double Tsync;
      //volatile double s1;
	//  volatile double s2;
	//  volatile double s3;
       int slaveAddrOffset;
       
		// how long is a slot?
        double slotDuration @unit(s) = default(100ms);
    	// schedule the second SYNC from node (i.e., rnode[0])
    	// duration between beacon (first SYNC packet) and second SYNC packet
    	double ScheduleOffset @unit(s) = default(0ms);
		
		
       
    gates:
        input in;
        input inclock;
        input inevent;
        output out;
        output outclock;
        
        input upperGateIn; 
        output upperGateOut;
}


