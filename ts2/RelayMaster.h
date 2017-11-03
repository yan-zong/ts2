//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Nov 2016
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

#ifndef RELAYMASTER_H_
#define RELAYMASTER_H_

#include <string.h>
#include <omnetpp.h>
#include "PtpPkt_m.h"
#include "Event_m.h"
#include "Constant.h"
#include "PCOClock.h"

class RelayMaster : public cSimpleModule
{
public:
    void startSync();

private:
    int myAddress;      // the variable is used in the multi-hop network
    int mySlaveAddress; // the variable is used in the multi-hop network
    double Tsync;
    double ScheduleRandomTime;  // random value generator
    double RandomTime;  // random value generator

protected:
    int lowerGateIn;
    int lowerGateOut;

    PCOClock *pClock; // pointer to clock module

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    cModule *findHost(void);

private:
    void handleSelfMessage(cMessage *msg);
    void handleSlaveMessage(PtpPkt *msg);

};

#endif /* RELAYMASTER_H_ */
