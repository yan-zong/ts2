//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  29 Jan 2014
// * Author:      Yiwen Huang, Xuweu Dai  (x.dai at ieee.org)
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
    double getTimestamp(); // now getTimestamp() is public
    void adjtimex(double value, int type); // now it public func
    void adj_offset_drift(); // now it public func
    void setT123(double t1, double t2, double t3)
    {
        Tm=t1;
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
