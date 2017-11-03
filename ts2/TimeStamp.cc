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

#include "TimeStamp.h"
#include "omnetpp.h"
#include "PtpPkt_m.h"

Define_Module(TimeStamp);

void TimeStamp::initialize()
{
    // Clock source selection
    // by default, if there is a clock module inside the node, use that clock as the time source
    //             if there is no a clock module, use the simTime() as the time source.

    ev<<"TimeStamp: initialize..."<<endl;
    pClk=(PCOClock *)(findHost()->getSubmodule("clock"));

    if (pClk==NULL) // no clock is found, force to use simTime() as the node's clock
    {
        useGlobalRefClock = true;
        ev<<"TimeStamp: No clock module is found. set useGlobalRefClock=true, use simTime() for time stamping\n";
    }
    else    // we have clock module, select time source
    {
        if (hasPar("useGlobalRefClock"))
        {
            useGlobalRefClock = par("useGlobalRefClock");
            ev<<"TimeStamp: useGlobalRefClock = "<<useGlobalRefClock;
            ev<<" , use parameter to determine clock source or time stamping\n";
        }
        else    //find clock module, but no parameter useGlobalRefClock is specified,
        {
            useGlobalRefClock = false;
            ev<<"TimeStamp: useGlobalRefClock= false\n";
         }
    }
}

void TimeStamp::handleMessage(cMessage *msg)
{
    /* The getEncapsulatedPacket() function returns
     *   a pointer to the encapsulated packet,
     *   or NULL if no packet is encapsulated.
     */
    cPacket *pck= static_cast<cPacket *>(msg);

    // Tx packet
    if(msg->arrivedOn("upperGateIn"))   // Tx packet from upper layer, all nodes should be a sub module of the simulation which has no parent module!!!
    {
        ev<<"Timestamp: TX packet ";
        while( pck->getEncapsulatedPacket()!= NULL )
        {
             pck = pck -> getEncapsulatedPacket();
        }
        // now pck points to the highest layer packet
        // check if it is PtpPkt
        if (dynamic_cast<PtpPkt *>(pck) != NULL)
        {
            EV<<"is a PtpPkt packet.";
            // set new timestamp
            EV<<"timestamp PtpPkt->Data was " << ((PtpPkt*)pck) -> getData() <<endl;
            if (useGlobalRefClock)
            {
                ((PtpPkt*)pck) -> setTsTx(SIMTIME_DBL(simTime()));
                EV<<", New timestamp (by simTime()) is " << ((PtpPkt*)pck) -> getTsTx()<<endl;
            }
            else
            {
                ((PtpPkt*)pck)->setTsTx(pClk->getTimestamp());
                ev<<", New timestamp (by clock module " << pClk -> getName() << ") is ";
                ev<< ((PtpPkt*)pck) -> getTsTx() <<endl;
            }
        }
        else
        {
            ev<<" is NOT a PtpPkt packet.Forward down to lower layer without time stamping."<<endl;
        }
        send(msg,"lowerGateOut");
        return;
    }

    // Rx packet
    if(msg->arrivedOn("lowerGateIn"))
    {
        ev << "Timestamp: RX packet..." << endl;

        while(pck->getEncapsulatedPacket() != NULL)
        {
            pck = pck->getEncapsulatedPacket();
        }
        if (dynamic_cast<PtpPkt *>(pck) != NULL)
        {
            // for PCO
            double receivedTime;
            receivedTime = pClk->getTimestamp();
            pClk->setReceivedSYNCTime(receivedTime);
            ev << "Timestamp: SYNC packet is received at " << receivedTime << " on Timastamp module. " << endl;

            /*
            // the encapsulated packet is a PtpPkt, put a time stamp
            double rxTimeStamp;
            ev<<" is a PtpPkt, its timestamp was "<< ((PtpPkt*)pck) -> getsetTsRx() <<endl;
            if (useGlobalRefClock)
            {
                rxTimeStamp = SIMTIME_DBL(simTime());
                ev << " Now using simTime() for new time stamp,\n";
            }
            else
            {
                rxTimeStamp = pClk -> getTimestamp();
                ev << " Now using clock module for new time stamp.\n";
            }

            ((PtpPkt*)pck)->setTsRx(rxTimeStamp);
            ev << " and new time stamp is "<<((PtpPkt*)pck)->getsetTsRx()<<endl;
            */
        }
        else
        {
            ev<<"  is NOT a PtpPkt, forward it to upper layer without time stamping"<<endl;
        }
        send(msg,"upperGateOut");
        return;
    }
}


cModule *TimeStamp::findHost(void)
{
    cModule *parent = getParentModule();
    cModule *node = this;

    // all nodes should be a sub module of the simulation which has no parent module!!!
    while( parent->getParentModule() != NULL ){
    node = parent;
    parent = node->getParentModule();
    }

    return node;
}
