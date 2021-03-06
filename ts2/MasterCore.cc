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

        // ---------------------------------------------------------------------------
        // ArpHost Module Parameters
        // parameter 'masterAddrOffset' is used to set the address of the master
        // set the master address, using the same IP, MAC address as ArpHost (see the *.ini file)
        // ---------------------------------------------------------------------------
        // Master address (only one master node is the network): 1000.
        if (hasPar("masterAddrOffset"))
            address = findHost() -> getIndex() + (int)par("masterAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
        else
            address = findHost() -> getIndex();

        ev<<"MasterCore: my address is "<< address << endl;

        // for PI controller
        TxThold = 0;
/*
        Packet * temp = new Packet("REGISTER");
        // we use the host modules findHost() as a appl address
        temp -> setDestination(PACKET_BROADCAST_ADDR);
        temp -> setSource(address);
        temp -> setPacketType(REG);

        temp -> setDestAddr(LAddress::L3BROADCAST);
        temp -> setSrcAddr( LAddress::L3Type(address));
        temp -> setByteLength(0);

        NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST);

        EV << "MasterCore: Core broadcasts REGISTER packet" << endl;
        send(temp, "lowerGateOut");
*/
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
        send(msg, "lowerGateOut");
    }

    else if (whichGate == lowerGateIn)
    {
        EV<<"MasterCore: receives a packet... "<<endl;

        if (dynamic_cast<Packet *>(msg) != NULL)
        {
            EV<< "MasterCore: this is a Packet, MasterCore is processing it now, "<<endl;
            Packet *pck = static_cast<Packet *>(msg);
            if((pck -> getSource() != address) &
               ((pck -> getDestination() == address) | (pck -> getDestination() == PACKET_BROADCAST_ADDR)))
             {
                EV << "MasterCore: the packet is for me, process it\n";
                handleSlaveMessage(pck); // handelSlaveMessage() does not delete msg
             }
             else
                EV << "MasterCore: the packet is not for me, ignore it\n";

             delete msg;
        }
        else
        {
            EV << "MasterCore: this is not a Packet, send it up to higher layer"<<endl;
            send(msg, "upperGateOut");
        }
    }

    else if (whichGate == inclock)
    {
        EV << "MasterCore: Master receives a SYNC packet from clock module, delete it and re-generate a full SYNC packet \n";
        TxThold = ((Packet*)msg) -> getData();
        delete msg;

        EV << "MasterCore: the threshold from 'PCOClock' module is " << TxThold <<endl;

        scheduleAt(simTime(), new cMessage("FireTimer"));

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
    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);

    pck -> setByteLength(TIMESTAMP_BYTE);

    pck -> setSource(address);
    pck -> setDestination(PACKET_BROADCAST_ADDR);

    pck -> setData(TxThold);
    // pck->setData(SIMTIME_DBL(simTime()));

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr( LAddress::L3Type(address));
    pck -> setDestAddr(LAddress::L3BROADCAST);

    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST);

    EV << "MasterCore: the threshold in SYNC packet is " << pck -> getData() <<endl;
    EV << "MasterCore: Master broadcasts SYNC packet" << endl;
    send(pck, "lowerGateOut");

}

void MasterCore::handleSlaveMessage(Packet *msg)
{
    switch (msg -> getPacketType())
    {
        case REG:
        {
            if ((msg -> getSource()) == 1000)
            {
                ev << "MasterCore: Register packet from master node, ignore it. \n";
                break;
            }

            else if (((msg -> getSource()) >= 2000) & ((msg -> getSource()) < 3000))
            {
                ev << "MasterCore: Register packet from relay, process it. \n";

                Packet *rplPkt= static_cast<Packet *>((Packet *)msg -> dup());
                rplPkt -> setName("REPLY_REGISGER");
                rplPkt -> setByteLength(0);
                rplPkt -> setPacketType(REGREP);

                rplPkt -> setSource(address);
                rplPkt -> setDestination(((Packet *)msg) -> getSource());

                rplPkt -> setSrcAddr(LAddress::L3Type(rplPkt -> getSource()));
                rplPkt -> setDestAddr(LAddress::L3Type(rplPkt -> getDestination()));

                NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt -> getDestination()));
                send(rplPkt, "lowerGateOut");

                ev << "MasterCore: REPLY_REGISGER packet is transmitted to relay node. \n";
                break;
            }

            else if ((msg -> getSource()) >= 3000)
            {
            ev << "MasterCore: Register packet from slave, process it. \n";

            Packet *rplPkt= static_cast<Packet *>((Packet *)msg -> dup());
            rplPkt -> setName("REPLY_REGISGER");
            rplPkt -> setByteLength(0);
            rplPkt -> setPacketType(REGREP);

            rplPkt -> setSource(address);
            rplPkt -> setDestination(((Packet *)msg) -> getSource());

            rplPkt -> setSrcAddr(LAddress::L3Type(rplPkt -> getSource()));
            rplPkt -> setDestAddr(LAddress::L3Type(rplPkt -> getDestination()));

            NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt -> getDestination()));
            send(rplPkt, "lowerGateOut");

            ev << "MasterCore: REPLY_REGISGER packet is transmitted to the slave node. \n";
            break;
            }
        }

        case REGREP:
        {
            if ((msg -> getSource()) == 1000)
            {
                ev << "MasterCore: Received register_reply packet from the master node, ignore it. \n";
                break;
            }
            else if (((msg -> getSource()) >= 2000) & ((msg -> getSource()) < 3000))
            {
                ev << "MasterCore: Received register_reply packet from the relay node, ignore it. \n";
                break;
            }
            else if ((msg -> getSource()) >= 3000)
            {
                ev << "MasterCore: Received register_reply packet to the slave node, ignore it. \n";
                break;
            }
        }

        case SYNC:
        {
            ev << "MasterCore: Core module receives a SYNC packet, ignore it.\n";
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

