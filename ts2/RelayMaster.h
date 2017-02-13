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

#ifndef PTPM2_H_
#define PTPM2_H_

//****************************************************************************
// * Brief:       This module provides the M2 operation of S1M2
//****************************************************************************

#include <string.h>
#include <omnetpp.h>
#include "PtpPkt_m.h"
#include "Event_m.h"
#include "Constant.h"
#include "Clock2.h"

class RelayMaster : public cSimpleModule
{
public:
    void startSync();   // Yan: start M2S2 time synchronisation

private:
    const char *name;
    // int address;
    int RelayIndex;
    int myAddress;      // the variable is used in the multi-hop network
    int mySlaveAddress; // the variable is used in the multi-hop network
    // int SlaveAddr;      // the variable is also used in the multi-hop network
    double Tsync;
    double nbReceivedDelayRequests;  // count the total number of received DelayRequest
    double nbSentSyncs;  // count the total number of sent Sync
    double nbSentDelayResponses;  // count the total number of sent DelayResponse
    double ScheduleRandomTime;  // random value generator
    double RandomTime;  // random value generator

protected:
    /** @brief gate id
     *  only data gates are used*/
    /*@{*/
    // int upperGateIn;
    // int upperGateOut;
    int lowerGateIn;
    int lowerGateOut;
    /*@}*/

    Clock2 *pClock; // Yan Zong: pointer to clock module


protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    cModule *findHost(void);

private:
    void handleSelfMessage(cMessage *msg);
    void handleSlaveMessage(PtpPkt *msg);

};

#endif /* PTPMASTER_H_ */
