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

#include "TDMAmac.h"
#include "FWMath.h"
#include "MacToPhyInterface.h"
#include "TDMAMacPkt_m.h"
#include "FindModule.h"

Define_Module(TDMAmac);

/* Initialize the mac using omnetpp.ini variables and initializing other necessary variables  */

void TDMAmac::initialize(int stage)
{
    BaseMacLayer::initialize(stage);

    if(stage == 0)
    {
        macPktQueue.clear();

        /* For droppedpacket code used #LMAC */
        BaseLayer::catDroppedPacketSignal.initialize();

        /* Getting parameters from ini file and setting MAC variables #LMAC */
        queueLength = par("queueLength");
        bitrate = par("bitrate");
        headerLength = par("headerLength");
        coreEV << "headerLength is: " << headerLength << endl;
        txPower = par("txPower");

        /* For dropped packets if required */
        droppedPacket.setReason(DroppedPacket::NONE);

        SyncStatus = true;

        trace = par("trace").boolValue();
        stats = par("stats").boolValue();

        droppedPackets = 0;

        if(trace && !gateway)
        {
            // record all packet arrivals
            vqLength.setName("Queue_Length_MAC");
        }
    }

    else if(stage == 1)
    {

        EV << "queueLength = " << queueLength
        << " bitrate = " << bitrate << endl;

        // Initialise the pointer to clock module
        pClock2 = (PCOClock *)findHost()->getSubmodule("clock");
        if (pClock2 == NULL)
            error("No clock module is found in the module");

        // FrameTimer = new cMessage( "frame-timer", 0 );
    }
}

/* Module destructor */
TDMAmac::~TDMAmac()
{
    // cancelAndDelete(FrameTimer);

    MacPktQueue::iterator it;
       for(it = macPktQueue.begin(); it != macPktQueue.end(); ++it) {
           delete (*it);
       }
       macPktQueue.clear();
}

/* Module destructor */
void TDMAmac::finish()
{
    BaseMacLayer::finish();

    if(stats && !gateway)
    {
        recordScalar("Mean queue length",qLength.getMean());
        recordScalar("Min queue length",qLength.getMin());
        recordScalar("Max queue length",qLength.getMax());
        recordScalar("Dropped packets",droppedPackets);
    }
}

/*
 * Handles packets from the upper layer and starts the process
 * to send them down. #LMAC
 */
void TDMAmac::handleUpperMsg(cMessage* msg){

    assert(dynamic_cast<cPacket*>(msg));

    EV << "Packet from upper layer" << endl;

    /* Casting upper layer message to mac packet format */
    TDMAMacPkt *mac = static_cast<TDMAMacPkt *>(encapsMsg(static_cast<cPacket*>(msg)));

    /* check the synchronisation state (mode 1 or mode 2) */
    if (SyncStatus == TRUE)
    {
        EV << "TDMAmac: in mode 1, send SYNC packet directly " << endl;

        phy->setRadioState(MiximRadio::TX);
        TDMAMacPkt* data = mac->dup();
        data->setKind(1);
        attachSignal(data);

        EV << "Sending down data packet\n";
        sendDown(data);
    }

    else
    {
        /* Queue is not full, put packet at the end of list */
        if (macPktQueue.size() <= queueLength)
        {
            macPktQueue.push_back(mac);
            EV << "packet put in queue\n  queue size: " << macPktQueue.size() << endl;
        }
        else
        {
           /* Queue is full, new packet has to be deleted */
           EV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
           mac->setName("MAC ERROR");
           mac->setKind(PACKET_DROPPED);
           sendControlUp(mac);
           droppedPacket.setReason(DroppedPacket::QUEUE);
           emit(BaseLayer::catDroppedPacketSignal, &droppedPacket);
           EV <<  "ERROR: Queue is full, forced to delete.\n";
           droppedPackets++;
        }
    }

    if(!gateway)
    {
        if(trace)
        {
            vqLength.record(macPktQueue.size());
        }
        qLength.collect(macPktQueue.size());
    }
}

/* Handles the messages sent to self-mainly timers */
void TDMAmac::handleSelfMsg(cMessage* msg)
{
      switch (msg->getKind())
      {
          /* SETUP phase enters to start the MAC protocol */
          case 0:
          {
              ;
          }
          break;

          case 1:
          {
              if(macPktQueue.empty())
                  break;

              phy->setRadioState(MiximRadio::TX);
              TDMAMacPkt* data = macPktQueue.front()->dup();
              data->setKind(1);
              attachSignal(data);
              coreEV << "Sending down data packet\n";
              sendDown(data);
          }
          break;

          default:
          {
              EV << "WARNING: unknown timer callback " << msg->getKind() << endl;
          }
    }
}

/*
 * Encapsulates the packet from the upper layer and
 * creates and attaches signal to it. #LMAC
 */
TDMAmac::macpkt_ptr_t TDMAmac::encapsMsg(cPacket* msg) {

    TDMAMacPkt *pkt = new TDMAMacPkt(msg->getName(), msg->getKind());
    pkt->setBitLength(headerLength);

    /*  copy dest address from the Control Info attached to the network message by the network layer #LMAC */
    cObject *const cInfo = msg->removeControlInfo();

    debugEV << "CInfo removed, mac addr=" << getUpperDestinationFromControlInfo(cInfo) << endl;
    pkt->setDestAddr(getUpperDestinationFromControlInfo(cInfo));

    /* delete the control info #LMAC */
    delete cInfo;

    /* set the src address to own mac address (nic module getId()) #LMAC */
    pkt->setSrcAddr(myMacAddr);

    /* encapsulate the network packet #LMAC */
    pkt->encapsulate(check_and_cast<cPacket *>(msg));
    debugEV <<"pkt encapsulated\n";

    return pkt;
}

/*
 * Handles received Mac packets from Physical layer. ASserts the packet
 * was received correct and checks if it was meant for us. #LMAC
 */
void TDMAmac::handleLowerMsg(cMessage* msg)
{
    TDMAMacPkt *const mac  = static_cast<TDMAMacPkt *>(msg);
    const LAddress::L2Type& dest = mac->getDestAddr();

    /*
     * Unused part (Collision tracking)
     * bool collision = false;
     * if we are listening to the channel and receive anything, there is a collision in the slot.
     * if (checkChannel->isScheduled())
     * {
     *  cancelEvent(checkChannel);
     *  collision = true;
     * }
     */

    /* Check if the packet is a broadcast or addressed to this node */
    EV << " I have received a data packet.\n";
    if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
    {
        EV << "sending pkt to upper...\n";
        sendUp(decapsMsg(mac));
    }
    else {
        EV << "packet not for me, deleting...\n";
        delete mac;
    }
}

void TDMAmac::handleLowerControl(cMessage* msg)
{
    switch(msg->getKind()) {
    case MacToPhyInterface::TX_OVER:
           debugEV << "PHY indicated transmission over" << endl;
           phy->setRadioState(MiximRadio::RX);

           if (SyncStatus == TRUE)
           {
               delete msg;
           }
           else
           {
               delete macPktQueue.front();
               macPktQueue.pop_front();
               delete msg;
           }
           break;
    default:{
        EV << "WARNING: unknown lower control " << msg->getKind() << endl;
    }
    }
}

/* Used by encapsulation function #LMAC */
void TDMAmac::attachSignal(macpkt_ptr_t macPkt)
{
    //calc signal duration
    simtime_t duration = macPkt->getBitLength() / bitrate;
    //create signal
    setDownControlInfo(macPkt, createSignal(simTime(), duration, txPower, bitrate));
}
