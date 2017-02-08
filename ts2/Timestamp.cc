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

#include "Timestamp.h"
#include "omnetpp.h"
#include "PtpPkt_m.h"


Define_Module(Timestamp);

void Timestamp::initialize()
{
    // Clock source selection
    // by default, if there is a clock module inside the node, use that clock as the time source
    //             if there is no a clock module, use the simTime() as the time source.
    ev<<"Timestamp: initialise "<<endl;
    pNode=(Slave *)(findHost()->getSubmodule("ptpCore"));
    pClk=(Clock2 *)(findHost()->getSubmodule("clock"));

    if (pClk == NULL)
    {
        // no clock is found, use simTime() as the node's clock
        useGlobalRefClock = true;
        ev<<" Timestamp: No clock module is found. set useGlobalRefClock is true, use simTime() for time-stamp\n";
     }
    else
    {
        // clock module found, select time source by clock module
        if (hasPar("useGlobalRefClock"))
        {
            useGlobalRefClock = par("useGlobalRefClock");
            ev<<" Timestamp: useGlobalRefClock = " <<useGlobalRefClock;
            ev<<" , use parameter to determine clock source or time-stamp\n";
        }
        else
        {
            // clock module found, but no parameter useGlobalRefClock is specificed,
            useGlobalRefClock = false;
            ev<<" Timestamp: useGlobalRefClock = false\n";
         }
    }
}

void Timestamp::handleMessage(cMessage *msg)
{
    // getEncapsulatedPacket() function returns a pointer to the encapsulated packet
    // or NULL if no packet is encapsulated.

    cPacket *pck= static_cast<cPacket *>(msg);

    // Tx packet
    if(msg -> arrivedOn("upperGateIn"))
    {
        // all nodes should be a sub module of the simulation which has no parent module!!!
        ev<<" the Tx packet is Tx packet from upper layer\n";
      while( pck->getEncapsulatedPacket()!= NULL )
      {
          pck = pck -> getEncapsulatedPacket();
      }
      // the pck points to the highest layer packet, and check whether it is PtpPkt
      if (dynamic_cast<PtpPkt *>(pck) != NULL)
      {
          EV<<" Tx packet is a PtpPkt packet\n";
         // PtpPkt *pck2= static_cast<PtpPkt *>(pck);
         // pck2->setData(0);
         // set new time stamp
          EV<<" timestamp PtpPkt -> Data was"<< ((PtpPkt*)pck) -> getData() <<endl;
          if (useGlobalRefClock)
           { // ((PtpPkt*)pck)->setData(SIMTIME_DBL(simTime()));
             ((PtpPkt*)pck)->setTsTx(SIMTIME_DBL(simTime()));
              EV<<"  New timestamp (by simTime()) is "<< ((PtpPkt*)pck) -> getData() <<endl;
            }
          else
            {
              // ((PtpPkt*)pck)->setData(pClk->getTimestamp());
              ((PtpPkt*)pck) -> setTsTx( pClk -> getTimestamp() );
              ev<<"  New timestamp (by clock module "<< pClk -> getName() << ") is ";
              ev<< ((PtpPkt*)pck) -> getData() <<endl;
           }
      }

      else
      {
          ev<<" the Tx packet is NOT a PtpPkt packet.Forward down to lower layer without timestamp."<<endl;
      }

      send(msg,"lowerGateOut");
      return;
    }

    // Rx packet
    if(msg -> arrivedOn("lowerGateIn"))
    {
        while( pck -> getEncapsulatedPacket() != NULL )
        {
            pck = pck->getEncapsulatedPacket();
        }
       if (dynamic_cast<PtpPkt *>(pck) != NULL)
       {
           ev<<" RX packet ";
           // the encapsulated packet is a PtpPkt, put a time stamp
           double rxTimeStamp;
           ev<<" is a PtpPkt, its timestamp was "<< ((PtpPkt*)pck) -> getData() <<endl;
           if (useGlobalRefClock)
           {
               rxTimeStamp = SIMTIME_DBL(simTime());
               ev<<" Now using simTime() for new time stamp,\n";
           }
           else
           {
               rxTimeStamp = pClk -> getTimestamp();
               ev<<" Now using clock module for new time stamp.\n";
             }
           // ((PtpPkt*)pck)->setData(rxTimeStamp);
           ((PtpPkt*)pck) -> setTsRx(rxTimeStamp);
           ev<<" and new time stamp is "<< ((PtpPkt*)pck) -> getData()<<endl;

       }

       else
       {
           ev<<"  is NOT a PtpPkt, forward it to upper layer without time stamping"<<endl;
       }

       send(msg,"upperGateOut");
      return;
    }
}

cModule *Timestamp::findHost(void)
{
    cModule *parent = getParentModule();
    cModule *node = this;

    // all nodes should be a sub module of the simulation which has no parent module!!!
    while( parent->getParentModule() != NULL )
    {
        node = parent;
        parent = node->getParentModule();
    }

    return node;
}
