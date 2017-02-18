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
    // for PTP
    double getTimestamp();  // timestamp clock
    void adjtimex(double value, int type); // adjust the local clock offset (0) or drift (1)
    void adj_offset_drift(); // adjust the drift and offset of local clock
    void setT123(double t1, double t2, double t3)
    {
        Tm=t1;
    }
    // for PCO
    void adjustThreshold();

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void updateDisplay();
    //virtual void openfile();
    //virtual void closefile();
private:
    // for PTP
    double Phyclockupdate();
    void   recordResult();
    // double getTimestamp(); // now getTimestamp() is public
    void preprocess_offset();
   //  void adjtimex(double value, int type); // now it public func
   // void adj_offset_drift(); // now it public func
    void movingfilter();
    void kalmanfilter();
    //void adjtimex(double value[2]);


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

    /*moving Filter*/
    double alpha;
    double beta;

    /*kalman filter parameter*/
    double A[2][2];
    double B[2][2];
    double H[2][2];
    double Q[2][2]; //sigma1,sigma2  过程激励噪声协方差矩阵
    double R[2][2]; //观测噪声协方差矩阵
    double xkhat[2][1]; //offset、dirft初始值
    double ukhat[2][1];//offset、dirft的估计值（观测值/校正值）的初始值
    double Pk[2][2]; //先验估计误差协方差矩阵
    double Kf[2][2];//卡尔曼增益

    /*PCO Parameter*/
    double RegisterThreshold;   // the threshold value of register
    double ThresholdAdjustValue;
    double ThresholdAdjustValuePrevious;
    double StandardTimePrevious;    // used to reset the clock
    int iStandardTime;  // used to reset the clock

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
};


#endif /* CLOCK2_H_ */
