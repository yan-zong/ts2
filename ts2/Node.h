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



#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include "PtpPkt_m.h"
#include "Packet_m.h" // for information exchange with manager
#include "Event_m.h"


#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "Clock2.h"

/**
 * @brief A PTP slave node
 *
 *
 * @author Xuewu Dai
 */
class Node: public cSimpleModule{
protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
	virtual void updateDisplay();
	cModule *findHost(void);
private:
	/*Dichiarazione metodi.*/
	void handleSelfMessage(cMessage *msg);
	void handleClockMessage(cMessage *msg);
	void handleMasterMessage(cMessage *msg);
	void ProduceT3packet();
	void recordResult();
	void handleOtherPacket(cMessage *msg);
	void handleEventMessage(cMessage *msg);
	void servo_clock();
	//void update_offset_previous(double value);


	/*Metodi ausiliari.*/

	/*Dichiarazione variabili.*/
	const char *name;
	int address;
	int master;
    LAddress::L3Type masterL3Addr;
    LAddress::L3Type myL3Addr;

    cModule *myMasterNode; // default value (at stage 1) is myself (this)
    Clock2 *pClock; // pointer to my clock module

    /* Definitions of variable for time synchronization.*/
	double Tcamp;       //t2��t3֮��Ĵ���ʱ�ӣ�0,10*Tcamp��
	//double Tsync;       //�Ƚ�ΪTsync��10Tcamp
	double ts2;	//Timestamp T2 stamped on receiving SYNC packet by slave
	double ts1;	//Timestamp T1 stamped on transmission SYNC packet by master
	double ts3;	//Timestamp T3 stamped on transmission DREQ packet by slave
	double ts4;	//Timestamp T4 stamped on receiving DRES packet by master
	double dprop;		//propogation delay
	double dms;			//master-to-slave delay for SYNC packet, dms = ts2-ts1
	                    //t2-t1��������Ĵ����ӳ٣�uniform(0,a),a=t2-t1
	double dsm;			//slave-to-master delay for DREQ packet, dsm=t4-t3
	double offset;		//slave's clock Offset (the difference between slave and master clocks)
	double drift;		//slave's clock drift (the frequency difference between slave and master clocks)
	double offset_previous;//ǰһʱ��ͬ������offset�Ĺ۲�ֵ

	/* Parameters for clock correction*/
	double Ts;  //
	double Ts_correct;
	double Ts_previous;
	double Tm;
	double Tm_previous;
	double  delay;//t2��t3֮���ʱ��
	//t4-t1��(t4-t1)<Tcamp<Tsync����֤ͬ��������һ��Tcamp����ɣ�������PTP��������һ�£���һ��ͬ�������У�drift��offset����
	double delta_t41;
	/* ������Ĵ���ʱ�ӵı���*/
	double rate;
	double T;
	double Tr;
	/*���� filter*/
   // double alpha;
    //double  beta;

	/* Vectors recording simulation results for performance analysis*/
	cOutVector dpropVec;
	cOutVector dmsVec;
	cOutVector dsmVec;
	cOutVector offsetVec;
	cOutVector driftVec;
	cOutVector delayVec;
	cOutVector delta_t41Vec;
	cOutVector TrVec; //��¼���Ĵ���ʱ��

};
