/*
Copyright(C) 2007 Giada Giorgi

This file is part of X-Simulator.

    X-Simulator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    X-Simulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Event_m.h"
#include "Constant.h"


class Nodo:public cSimpleModule{
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
    cModule *myMasterNode; // default value (at stage 1) is myself (this)

	/* Variabili algoritmo di sincronizzazione.*/
	// Variabili definite dal protocollo PTP
	//double Tsync;		//Periodo con cui vengono inviati i messaggi di SYNC
	double Tcamp;       //t2、t3之间的处理时延（0,10*Tcamp）
	//double Tsync;       //比较为Tsync和10Tcamp
	double ts_s_sync;	//Timestamp assegnato dallo slave al msg SYNC
	double ts_m_sync;	//Timestamp assegnato dal master al msg SYN
	double ts_s_dreq;	//Timestamp assegnato dallo slave al msg DREQ
	double ts_m_dreq;	//Timestamp assegnato dal master al msg DREQ
	double dprop;		//Ritardo di propagazione one-way-delay
	double dms;			//Ritardo di propagazione tra master e slave //t2-t1，计算包的传输延迟，uniform(0,a),a=t2-t1
	double dsm;			//Ritardo di propagazione tra slave e master
	double offset;		//Offset misurato tra master e slave
	double drift;		//Drift misurato tra master e slave
	double offset_previous;//前一时刻同步周期offset的观测值

	/* Parametri servo clock*/
	double Ts;
	double Ts_correct;
	double Ts_previous;
	double Tm;
	double Tm_previous;
	double  delay;//t2、t3之间的时延
	//t4-t1，(t4-t1)<Tcamp<Tsync，保证同步过程在一个Tcamp内完成，这样与PTP假设条件一致，在一个同步过程中，drift和offset不变
	double delta_t41;
	/* 计算包的传输时延的变量*/
	double rate;
	double T;
	double Tr;
	/*滑动 filter*/
   // double alpha;
    //double  beta;

	/* Variabili di uscita */
	cOutVector dpropVec;
	cOutVector dmsVec;
	cOutVector dsmVec;
	cOutVector offsetVec;
	cOutVector driftVec;
	cOutVector delayVec;
	cOutVector delta_t41Vec;
	cOutVector TrVec; //计录包的传输时延

};
