//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Nov 2016
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

package mytdmamac.TDMAmac;

simple PCOClock
{
    parameters:
        volatile double offset;	// the clock offset
        volatile double drift;	// the clock skew (the variation of clock frequency)
        volatile double sigma1;	// the standard deviation of clock skew noise
        volatile double sigma2;	// the standard deviation of clock offset noise
        volatile double sigma3;	// the standard deviation of timestamp (meausmrenet) noise
        volatile double u3;	// the mean of timestamp (measurement) noise
        volatile double tau_0;	// clock update period
        volatile double alpha;	// the correction parameter of clock offset
        volatile double beta;	// the correction parameter of clock skew        
        volatile double Threshold;
        double pulseDuration @unit(s) = default(0ms);	// duration between neighboring SYNC packets
    	double ScheduleOffset @unit(s) = default(0ms);
    	double tau @unit(s) = default(0ms);	// @brief transmission delay, see clock.h
    	int CorrectionAlgorithm;	// correction algorithm, 0 is for null
    								// 1 is for classic PCO by using constant value, 2 is for classic PCO by using offset value
    	double varepsilon = default(0);
    	int numRelay = default(0);      
    	    	
    gates:
        input inclock;
        output outclock;
}

