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

#include "PtpTimeStmp.h"
#include "omnetpp.h"
#include "PtpPkt_m.h"



Define_Module(PtpTimeStmp);

void PtpTimeStmp::initialize()
{
    // Clock source selection
    // by default, if there is a clock module inside the node, use that clock as the time source
    //             if there is no a clock module, use the simTime() as the time source.
    ev<<"PtpTimeStmp::initialize()"<<endl;
    pNode=(Node *)(findHost()->getSubmodule("ptpCore"));
    pClk=(Clock2 *)(findHost()->getSubmodule("clock"));

    if (pClk==NULL)
    { // no clock is found, force to use simTime() as the node's clock
        useGlobalRefClock=true;
        ev<<"    PtpTimeStmp::Initilize() No clock module is found. set useGlobalRefClock=true, use simTime() for time stamping\n";
     }
    else
    {  // we have clock module, select time source by
        if (hasPar("useGlobalRefClock"))
        {   useGlobalRefClock=par("useGlobalRefClock");
            ev<<"    PtpTimeStmp::Initilize() useGlobalRefClock="<<useGlobalRefClock;
            ev<<" , use parameter to determine clock source or time stamping\n";
        }
        else
        { //find clock module, but no parameter useGlobalRefClock is pscefied,
            useGlobalRefClock=false;
            ev<<"    PtpTimeStmp::Initilize() useGlobalRefClock= false\n";

         }

    }
}

void PtpTimeStmp::handleMessage(cMessage *msg)
{
    /* The getEncapsulatedPacket() function returns
     *   a pointer to the encapsulated packet,
     *   or NULL if no packet is encapsulated.
     */
    cPacket *pck= static_cast<cPacket *>(msg);

    // Tx packet
    if(msg->arrivedOn("upperGateIn"))
    {  // Tx packet from upper layer,
       // all nodes should be a sub module of the simulation which has no parent module!!!
        ev<<"   TX packet ";
      while( pck->getEncapsulatedPacket()!= NULL ){
             pck = pck->getEncapsulatedPacket();
          }
      // now pck points to the highest layer packet
      // check if it is PtpPkt
      if (dynamic_cast<PtpPkt *>(pck) != NULL)
      {  EV<<"is a PtpPkt packet.";
         // PtpPkt *pck2= static_cast<PtpPkt *>(pck);
         // pck2->setData(0);
         // set new time stamp
          EV<<"timestamp PtpPkt->Data was"<<((PtpPkt*)pck)->getData()<<endl;
          if (useGlobalRefClock)
           { // ((PtpPkt*)pck)->setData(SIMTIME_DBL(simTime()));
             ((PtpPkt*)pck)->setTsTx(SIMTIME_DBL(simTime()));
              EV<<"  New timestamp (by simTime()) is "<<((PtpPkt*)pck)->getData()<<endl;
            }
          else
            {
              // ((PtpPkt*)pck)->setData(pClk->getTimestamp());
              ((PtpPkt*)pck)->setTsTx(pClk->getTimestamp());
                  ev<<"  New timestamp (by clock module "<<pClk->getName()<< ") is ";
                  ev<<((PtpPkt*)pck)->getData()<<endl;
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
        ev << "Timestamp: RX packet: " << endl;

        while(pck->getEncapsulatedPacket() != NULL)
        {
            pck = pck->getEncapsulatedPacket();
        }
        if (dynamic_cast<PtpPkt *>(pck) != NULL)
        {
            double driftClock;
            driftClock = pClk->getTimestamp();
            pClk->setReceivedTime(driftClock);
            ev << "Timestamp: SYNC packet is received at " << driftClock << " on Timastamp module. " << endl;

            // for PTP
            /*
            // the encapsulated packet is a PtpPkt, put a time stamp
            double rxTimeStamp;
            ev<<" is a PtpPkt, its timestamp was "<<((PtpPkt*)pck)->getData()<<endl;
            if (useGlobalRefClock)
            {
                rxTimeStamp=SIMTIME_DBL(simTime());
                ev << " Now using simTime() for new time stamp,\n";
            }
            else
            {
                rxTimeStamp=pClk->getTimestamp();
                ev << " Now using clock module for new time stamp.\n";
            }

            // ((PtpPkt*)pck)->setData(rxTimeStamp);
            ((PtpPkt*)pck)->setTsRx(rxTimeStamp);
            ev << " and new time stamp is "<<((PtpPkt*)pck)->getData()<<endl;

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


cModule *PtpTimeStmp::findHost(void)
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
