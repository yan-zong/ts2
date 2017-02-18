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
    double drift_previous; //�������������Ϊ�˼���drift10-drift0��ֵ
    double offset_previous;
    double delta_drift;  //����drift10-drift0��ֵ
    double delta_offset;
    double offset_adj_value;//offset��У��ֵ
    double offset_adj_previous;//ǰһ��ͬ��ʱoffset��У��ֵ
    double drift_adj_value;
    double sim_time_limit;  //����ʱ��
    int i;
    int j;//ȥ��ǰ10��ͬ��У��ֵ
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
    double Q[2][2]; //sigma1,sigma2  ���̼�������Э�������
    double R[2][2]; //�۲�����Э�������
    double xkhat[2][1]; //offset��dirft��ʼֵ
    double ukhat[2][1];//offset��dirft�Ĺ���ֵ���۲�ֵ/У��ֵ���ĳ�ʼֵ
    double Pk[2][2]; //����������Э�������
    double Kf[2][2];//����������

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
    cOutVector update_numberVec;  // ��¼����ʱ�Ӹ��µĴ���
    cOutVector delta_driftVec; // ��¼drift10-drift0��ֵ
    cOutVector delta_offsetVec;
    cOutVector drift_adj_valueVec ;//��¼�˲����У��ֵ
    cOutVector offset_adj_valueVec;
    cOutVector drift_valueVec;    //ͬ��У��offsetʱ����¼offset��ʵֵ
    cOutVector offset_valueVec;   //ͬ��У��driftʱ����¼drift��ʵֵ
    cOutVector error_driftVec;   // ��¼driftͬ������ʸ����
    cOutVector error_offsetVec; // ��¼offsetͬ������ʸ����
    cStdDev    driftStd;           //Ϊ����drift��ƽ��ֵ�ͱ�׼��
    cStdDev    offsetStd;
    cStdDev    error_sync_drift;  // Ϊ����driftͬ�����ƽ��ֵ�ͱ�׼��
    cStdDev    error_sync_offset;     // Ϊ����offsetͬ�����ƽ��ֵ�ͱ�׼��
};


#endif /* CLOCK2_H_ */
