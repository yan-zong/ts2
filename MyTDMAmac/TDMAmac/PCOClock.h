//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Nov 2016
// * Author:      Yan Zong, Xuewu Dai
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
        double drift_present;  // the present clock skew
        double sigma1;  // the standard deviation of clock skew noise
        double sigma2;  // the standard deviation of clock offset noise
        double sigma3;  // the standard deviation of timestamp (meausmrenet) noise
        double u3;  // the mean of timestamp (measurement) noise
        double noise1;  // clock skew noise
        double noise2;  // clock offset noise
        double noise3;  // timestamp (meausmrenet) noise
        double tau_0;   // clock update period
        // double ReferenceClock;    // the classic clock time
        double ClassicClock;    // the classic clock time
        double PCOClockState;    // the PCO clock state
        double Threshold;   // PCO clock threshold
        double LastUpdateTime;  // store the last update time of reference time
        double Timestamp;   // timestamp
        double ReceivedSYNCTime;    // the time that node receives the SYNC packet
        double tau; // the transmission delay
        double alpha;  // the correction parameter of clock offset
        double beta;   // the correction parameter of clock skew
        int CorrectionAlgorithm;    // correction algorithm
                                    // 1 is for classic PCO by using constant value, 2 is for classic PCO by using offset value
        double varepsilon;  // the coupling strength of PCO model
        int numRelay;
        double LastFireTime;
        double refractory;
        double SumThreshold;

        /* @brief the id of node */
        int NodeId;

        /* @brief duration of SYNC packet from neighboring relay nodes */
        double pulseDuration;

        /* @brief duration of ScheduledOffset */
        double ScheduleOffset;

        int i;

        /* @brief parameters in PI controller */
        double offsetOutPContr = 0; // for P controller
        double skewOutPContr = 0;   // for P controller

        double offsetOutIContr = 0; // for I controller
        double skewOutIContr = 0;   // for I controller

        double offsetOutPIContr = 0;    // for PI controller
        double skewOutPIContr = 0;  // for PI controller

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
        cOutVector measuredoffset;
        cOutVector measuredskew;

        cStdDev    driftStd;    // standard deviation of skew
        cStdDev    offsetStd;   // standard deviation of offset
};

#endif /* PCOClock_H_ */
