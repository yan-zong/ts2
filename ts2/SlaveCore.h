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

#ifndef SLAVE_H_
#define SLAVE_H_

#include <string.h>
#include <math.h>
#include <omnetpp.h>
#include "Packet_m.h" // for information exchange with manager
#include "Event_m.h"
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "PCOClock.h"

class SlaveCore: public cSimpleModule
{
    private:
        int address;
        int myMasterAddress;
        int myRelayAddress;
        LAddress::L3Type masterL3Addr;
        LAddress::L3Type myL3Addr;
        double EstimatedOffset; // the measurement offset
        double EstimatedSkew; // the measurement skew
        int AddressOffset;
        double TxThold;
        double RxThold;

        PCOClock *pClock; // pointer to my clock module

    protected:
        int upperGateIn;
        int upperGateOut;
        int lowerGateIn;
        int lowerGateOut;
        int inclock;
        // int outclock;
        // int inevent;

    protected:
	    virtual void initialize();
	    virtual void handleMessage(cMessage *msg);
	    virtual void finish();
	    virtual void updateDisplay();
	    cModule *findHost(void);

    private:
	    void handleSelfMessage(cMessage *msg);
	    void handleMasterMessage(cMessage *msg);
	    void recordResult();
	    void handleOtherPacket(cMessage *msg);
	    void handleEventMessage(cMessage *msg);
	    void servo_clock();

};

#endif /* SLAVE_H_ */
