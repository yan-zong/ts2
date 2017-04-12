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


#ifndef PCOClock_H_
#define PCOClock_H_

#include <string.h>
#include <omnetpp.h>
#include <fstream>
#include "Packet_m.h"

class PCOClock:public cSimpleModule
{
    public:
        double getThresholdOffsetWithMaster();
        double getThresholdOffsetWithRelay();
        void adjustThresholdBasedMaster();
        void adjustThresholdBasedRelay();
        // int getnumPulse();
        // double getPCOTimestamp();
        double getTimestamp();  // timestamp clock
        double setReceivedTime(double value);

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void finish();
        virtual void updateDisplay();
        cModule *findHost(void);

    private:
        double physicalClockUpdate();
        void   recordResult();
        void generateSYNC();
        double IIRFilterMaster(double IIRFilterInput);
        double IIRFilterRelay(double IIRFilterInput);
        double AbsoluteValue(double AbsoultValueInput);

        // double lastupdatetime;
        // double phyclock;
        double softclock;
        double softclock_t2;
        double softclock_t3;
        double offset;
        double drift;
        double error_offset;
        double error_drift;
        double sigma1;
        double sigma2;
        double sigma3;
        double u3;
        double noise1;
        double noise2;
        double noise3;
        double Tcamp;
        double Tsync;
        double t_previous;
        double drift_previous; //定义这个变量是为了计算drift10-drift0的值
        double offset_previous;
        double delta_drift;  //计算drift10-drift0的值
        double delta_offset;
        double offset_adj_value;//offset的校正值
        double offset_adj_previous;//前一次同步时offset的校正值
        double drift_adj_value;
        double sim_time_limit;  //仿真时间
        int i;
        int j;//去掉前10个同步校正值
        int k;
        double Tm;
        double Tm_previous;


        /*PCO Parameter*/


        double ReferenceClock;
        double PhysicalClock;
        double Threshold;
        double ThresholdTotal;
        double LastUpdateTime;
        double PCOClock;

        // the parameter of numerator of IIR filter
        double IIRnum1;
        double IIRnum2;
        double IIRnum3;
        // the parameter of denominator of IIR filter
        double IIRden1;
        double IIRden2;
        double IIRden3;

        double IIRFilterOutputMaster;
        double IIRFilterOutputRelay;

        double IIRInputMaster1 = 0;
        double IIRInputMaster2 = 0;
        double IIRInputMaster3 = 0;
        double IIROutputMaster1 = 0;
        double IIROutputMaster2 = 0;
        double IIROutputMaster3 = 0;

        double IIRInputRelay1 = 0;
        double IIRInputRelay2 = 0;
        double IIRInputRelay3 = 0;
        double IIROutputRelay1 = 0;
        double IIROutputRelay2 = 0;
        double IIROutputRelay3 = 0;


        //_____________________


        // double RegisterThreshold;   // the threshold value of register
        double PulseTimePrevious;    // used to reset the clock
        // int numPulse;
        double FrameDuration;
        // double LastUpdateTime;

        double offsetTotal;
        double RefTimePreviousPulse;
        double ReceivedPulseTime;

        double NormalizedReceivedPulseTime;
        double NormalizedThreshold;

        double ThresholdOffsetBasedMaster;
        double ThresholdOffsetBasedRelay;

        double ThresholdOffsetPreviousBasedMaster;
        double ThresholdOffsetPreviousBasedRelay;

        double ThresholdAdjustValueBasedMaster;
        double ThresholdAdjustValueBasedRelay;

        double ThresholdAdjustValueBasedMasterIIR;
        double ThresholdAdjustValueBasedRelayIIR;

        int CorrectionAlgorithm;

        /* @brief this delay consists of transmission delay and propagation delay
         * for propagation delay, the time for 50m is 1/6us
         * for transmission delay, the time for one SYNC packet (44 bytes) is 1.408ms
         * the propagation delay is negligible */
        double delay;

        /* @breif the offset between the PCO drifting clock and standard clock */
        // double ClockOffset;

        /* @brief the id of node */
        int NodeId;

        /* @brief Duration of a slot #LMAC */
        double slotDuration;

        /* @brief schedule the second SYNC from node (i.e., rnode[0]) */
        /* @brief duration between beacon (first SYNC packet) and second SYNC packet */
        double ScheduleOffset;

        double AdjustParameter;

        /* @brief time of generating pulse (simTime) */
        double PulseTime;


        std::ofstream outFile;
        cOutVector softclockVec;
        cOutVector softclock_t2Vec;
        cOutVector softclock_t3Vec;
        cOutVector noise1Vec;
        cOutVector noise2Vec;
        cOutVector noise3Vec;
        cOutVector driftVec;
        cOutVector offsetVec;
        cOutVector update_numberVec;  // 记录物理时钟更新的次数
        cOutVector delta_driftVec; // 记录drift10-drift0的值
        cOutVector delta_offsetVec;
        cOutVector drift_adj_valueVec ;//记录滤波后的校正值
        cOutVector offset_adj_valueVec;
        cOutVector drift_valueVec;    //同步校正offset时，记录offset真实值
        cOutVector offset_valueVec;   //同步校正drift时，记录drift真实值
        cOutVector error_driftVec;   // 记录drift同步误差的矢量类
        cOutVector error_offsetVec; // 记录offset同步误差的矢量类
        cStdDev    driftStd;           //为计算drift的平均值和标准差
        cStdDev    offsetStd;
        cStdDev    error_sync_drift;  // 为计算drift同步误差平均值和标准差
        cStdDev    error_sync_offset;     // 为计算offset同步误差平均值和标准差

        cOutVector adjustedthresholdvalueVec;    // the adjusted value of threshold
        cOutVector thresholdVec;    // the threshold value
        // cOutVector phyclockVec;
        cOutVector pulsetimeVec;    // the threshold value
        // cOutVector thresholdOffsetVec;

        cOutVector thresholdOffsetWithMasterVec;
        cOutVector thresholdOffsetWithrelayVec;

        cOutVector thresholdOffsetWithMasterIIRVec;
        cOutVector thresholdOffsetWithrelayIIRVec;

        cOutVector physicalClockVec;
        // cOutVector offsetTotalVec;
};

#endif /* PCOClock_H_ */
