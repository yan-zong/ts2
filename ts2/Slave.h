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

#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include "PtpPkt_m.h"
#include "Packet_m.h" // for information exchange with manager
#include "Event_m.h"
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "PCOClock.h"

class Slave: public cSimpleModule
{
    protected:
	    virtual void initialize();
	    virtual void handleMessage(cMessage *msg);
	    virtual void finish();
	    virtual void updateDisplay();
	    cModule *findHost(void);

    private:
	    void handleSelfMessage(cMessage *msg);
	    void handleClockMessage(cMessage *msg);
	    void handleMasterMessage(cMessage *msg);
	    void ProduceT3packet();
	    void recordResult();
	    void handleOtherPacket(cMessage *msg);
	    void handleEventMessage(cMessage *msg);
	    void servo_clock();

	int address;
	int master;
    LAddress::L3Type masterL3Addr;
    LAddress::L3Type myL3Addr;

    cModule *myMasterNode; // default value (at stage 1) is myself (this)
    PCOClock *pClock; // pointer to my clock module

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

	/* Parameters for clock correction */
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

	/* Vectors recording simulation results for performance analysis */
	cOutVector dpropVec;
	cOutVector dmsVec;
	cOutVector dsmVec;
	cOutVector offsetVec;
	cOutVector driftVec;
	cOutVector delayVec;
	cOutVector delta_t41Vec;
	cOutVector TrVec; //��¼���Ĵ���ʱ��

};
