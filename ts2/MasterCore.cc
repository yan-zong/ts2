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

#include "MasterCore.h"

#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"

Define_Module(MasterCore);

void MasterCore::initialize()
{
        upperGateIn  = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
        lowerGateIn  = findGate("lowerGateIn");
        lowerGateOut = findGate("lowerGateOut");
        inclock = findGate ("inclock");
        // outclock = findGate ("outclock");

        if (hasPar("masterAddrOffset"))
            address = findHost() -> getIndex() + (int)par("masterAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
        else
            address = findHost() -> getIndex();

        ev<<"MasterCore: my address is "<< address << endl;

        PtpPkt * temp = new PtpPkt("REGISTER");
        // we use the host modules findHost() as a appl address
        temp->setDestination(PTP_BROADCAST_ADDR);
        temp->setSource(address);
        temp->setPtpType(REG);

        temp->setDestAddr(LAddress::L3BROADCAST);
        temp->setSrcAddr( LAddress::L3Type(address));
        temp->setByteLength(0);

        NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

        EV << "MasterCore: Core broadcasts REGISTER packet" << endl;
        send(temp,"lowerGateOut");
  }

void MasterCore::handleMessage(cMessage* msg)
{
    int whichGate;

    if (msg -> isSelfMessage())
    {
        handleSelfMessage(msg);
        return;
    }

    // not self-message,check where it comes from
    whichGate = msg -> getArrivalGateId();

    if(whichGate == upperGateIn)
    {
        send(msg,"lowerGateOut");
    }

    else if(whichGate == lowerGateIn)
    {
        // check PtpPkt type
        EV<<"MasterCore Receives a packet"<<endl;

        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {
            EV<< " MasterCore: This ia a PtpPkt packet, MasterCore is processing it now"<<endl;
            PtpPkt *pck= static_cast<PtpPkt *>(msg);
            if(pck -> getSource() != address &
               (pck -> getDestination() == address | pck -> getDestination() == PTP_BROADCAST_ADDR))
             {
                EV << "the packet is for me, process it\n";
                handleSlaveMessage(pck); // handelSlaveMessage() does not delete msg
             }
             else
                EV << "the packet is not for me, ignore it\n";

             delete msg;
        }
        else
        {
            EV << "This is not a PtpPkt packet, send it up to higher layer"<<endl;
            send(msg,"upperGateOut");
        }
    }

    else if(whichGate == inclock)
    {
        EV << "MasterCore: Master receives a SYNC packet from clock module, delete it and re-generate a full SYNC packet \n";
        delete msg;

        scheduleAt(simTime(), new cMessage("FrameTimer"));

    }

    else if(whichGate == -1)
    {
        /* Classes extending this class may not use all the gates, f.e.
         * BaseApplLayer has no upper gates. In this case all upper gate-
         * handles are initialized to -1. When getArrivalGateId() equals -1,
         * it would be wrong to forward the message to one of these gates,
         * as they actually don't exist, so raise an error instead.
         */
        opp_error("No self message and no gateID?? Check configuration.");
    }

    else
    {
        /* msg->getArrivalGateId() should be valid, but it isn't recognized
         * here. This could signal the case that this class is extended
         * with extra gates, but handleMessage() isn't overridden to
         * check for the new gate(s).
         */
        opp_error("Unknown gateID?? Check configuration or override handleMessage().");
    }
}

void MasterCore::handleSelfMessage(cMessage *msg)
{
    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);

    pck->setByteLength(44);
    // pck->setTimestamp(simTime());

    pck->setSource(address);
    pck->setDestination(PTP_BROADCAST_ADDR);

    // pck->setData(SIMTIME_DBL(simTime()));
    // pck->setTsTx(SIMTIME_DBL(simTime())); // set transmission timie stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setSrcAddr( LAddress::L3Type(address));
    pck->setDestAddr(LAddress::L3BROADCAST);

    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "MasterCore: Master broadcasts SYNC packet" << endl;
    send(pck,"lowerGateOut");

}

void MasterCore::handleSlaveMessage(PtpPkt *msg)
{
    switch (msg -> getPtpType())
    {
        case SYNC:
        {
            ev << "MasterCore: MasterCore receives a SYNC packet, ignore it.\n";
            break;
        }
        case REG:
        {
             PtpPkt *rplPkt= static_cast<PtpPkt *>((PtpPkt *)msg->dup());
             rplPkt->setName("REPLY_REGISGER");
             rplPkt->setByteLength(0);
             rplPkt->setPtpType(REGREPLY);

             rplPkt->setSource(address);
             rplPkt->setDestination(((PtpPkt *)msg)->getSource());

             rplPkt->setSrcAddr(LAddress::L3Type(rplPkt->getSource()));
             rplPkt->setDestAddr(LAddress::L3Type(rplPkt->getDestination()));

             NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt->getDestination()));
             send(rplPkt, "lowerGateOut");
             break;
        }
        case REGRELAY:
        {
            ev << "MasterCore: Register packet from relay node, ignore it\n";
            break;
        }
        case REGREPLY:
        {
            ev << "MasterCore: Received register_reply packet (PtpType = REGREPLY), ignore\n";
            break;
        }
        default:
        {
            ev << "MasterCore: unknown message. Report warning and ignore\n";
            break;
        }
    }
}

void MasterCore::finish()
{
}

cModule *MasterCore::findHost(void)
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

