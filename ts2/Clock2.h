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

#ifndef CLOCK2_H_
#define CLOCK2_H_

#include <string.h>
#include <omnetpp.h>
#include <fstream>
#include "Packet_m.h"

// #include <stdio.h>
//  #include <iostream>
//  #include <fstream>

// using namespace std;

class Clock2:public cSimpleModule{
public:
    double getTimestamp();
    void adjtimex(double value, int type);
    void adj_offset_drift();
    void setT123(double t1, double t2, double t3)
    {
        Tm = t1;
    }

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void updateDisplay();
    //virtual void openfile();
    //virtual void closefile();
private:
    double Phyclockupdate();
    void   recordResult();
    void preprocess_offset();
    void movingfilter();
    void kalmanfilter();
    double lastupdatetime;
    double phyclock;
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
    double drift_previous;  // the value of drift10-drift0
    double offset_previous;
    double delta_drift; // the value of drift10-drift0
    double delta_offset;
    double offset_adj_value;    // the value of adjustment for offset
    double offset_adj_previous; // the previous value of adjustment for offset
    double drift_adj_value;
    double sim_time_limit;  // the time of simulation
    int i;
    int j;  //remove 10 previous value of adjustment for offset
    int k;
    double Tm;
    double Tm_previous;

    // moving filter
    double alpha;
    double beta;

    // kalman filter parameter
    double A[2][2];
    double B[2][2];
    double H[2][2];
    double Q[2][2]; //sigma1,sigma2  过程激励噪声协方差矩阵
    double R[2][2]; //观测噪声协方差矩阵
    double xkhat[2][1]; //offset、dirft初始值
    double ukhat[2][1];//offset、dirft的估计值（观测值/校正值）的初始值
    double Pk[2][2]; //先验估计误差协方差矩阵
    double Kf[2][2];//卡尔曼增益

    std::ofstream outFile;
    cOutVector softclockVec;
    cOutVector softclock_t2Vec;
    cOutVector softclock_t3Vec;
    cOutVector noise1Vec;
    cOutVector noise2Vec;
    cOutVector noise3Vec;
    cOutVector driftVec;
    cOutVector offsetVec;
    cOutVector update_numberVec;    // the time of physical clock update
    cOutVector delta_driftVec;  // the value of drift10-drift0
    cOutVector delta_offsetVec;
    cOutVector drift_adj_valueVec;  //记录滤波后的校正值
    cOutVector offset_adj_valueVec;
    cOutVector drift_valueVec;    // the real value of adjusted offset
    cOutVector offset_valueVec;   // the real value of adjusted drift
    cOutVector error_driftVec;   // the vector for synchronised error of drift
    cOutVector error_offsetVec; // the vector for synchronised error of offset
    cStdDev    driftStd;           // the average and standard derivation of drift
    cStdDev    offsetStd;
    cStdDev    error_sync_drift;  // the average and standard derivation for synchronised error of drift
    cStdDev    error_sync_offset;   // the average and standard derivation for synchronised error of offset
};


#endif /* CLOCK2_H_ */
