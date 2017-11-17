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

#ifndef PCOClock_H_
#define PCOClock_H_

#include <string.h>
#include <omnetpp.h>
#include <fstream>
#include "Packet_m.h"

class PCOClock:public cSimpleModule
{
    public:
        double getMeasurementOffset(int MeasurmentAlgorithm, int AddressOffset);
        double getMeasurementSkew(double measuredOffset);
        void adjustClock(double estimatedOffset, double estimatedSkew);
        double getTimestamp();
        double setReceivedSYNCTime(double value);

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void finish();
        virtual void updateDisplay();
        cModule *findHost(void);

    private:
        double ClockUpdate();
        void recordResult();
        void generateSYNC();
        double AbsoluteValue(double AbsoultValueInput);

        double offset;  // the clock offset
        double drift;   // the clock skew (the variation of clock frequency)
        double offset_present; // the present clock offset
        double drift_present;  // the present clock skew
        double sigma1;  // the standard deviation of clock skew noise
        double sigma2;  // the standard deviation of clock offset noise
        double sigma3;  // the standard deviation of timestamp (meausmrenet) noise
        double u3;  // the mean of timestamp (measurement) noise
        double noise1;  // clock skew noise
        double noise2;  // clock offset noise
        double noise3;  // timestamp (meausmrenet) noise
        double tau_0;   // clock update period
        double ClassicClock;    // the classic clock time
        double PCOClockState;    // the PCO clock state
        double Threshold;   // PCO clock threshold
        double LastUpdateTime;  // used to store the last update time of reference time
        double Timestamp;   // timestamp based on the reception of SYNC packet
        double ReceivedSYNCTime;    // the time that node receives the SYNC packet
        double tau; // the transmission delay
        double alpha;  // the correction parameter of clock offset
        double beta;   // the correction parameter of clock skew
        int CorrectionAlgorithm;    // correction algorithm
                                    // 1 is for classic PCO by using constant value, 2 is for classic PCO by using offset value
        double varepsilon;
        int numRelay;

        /* @brief the id of node */
        int NodeId;

        /* @brief Duration of a slot #LMAC */
        // double slotDuration; // yan zong
        double pulseDuration;

        /* @brief schedule the second SYNC from node (i.e., rnode[0]) */
        /* @brief duration between beacon (first SYNC packet) and second SYNC packet */
        double ScheduleOffset;

        int i;
        double PCOfireTime;


        std::ofstream outFile;
        cOutVector noise1Vec;
        cOutVector noise2Vec;
        cOutVector noise3Vec;
        cOutVector driftVec;
        cOutVector offsetVec;
        cOutVector update_numberVec;  // the clock update times
        cOutVector thresholdVec;    // the threshold value
        cOutVector measuredoffsetmasterVec;
        cOutVector measuredoffsetrelayVec;
        cOutVector classicclockVec;
        cOutVector pcoclockVec;
        cOutVector timestampVec;
        cOutVector PCOfireTimeVec;

        cStdDev    driftStd;    // standard deviation of skew
        cStdDev    offsetStd;   // standard deviation of offset
};

#endif /* PCOClock_H_ */
