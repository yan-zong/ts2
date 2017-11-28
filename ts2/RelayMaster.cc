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

#include "RelayMaster.h"
#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "distrib.h"

Define_Module(RelayMaster);

void RelayMaster::initialize()
{
        ev << "RelayMaster: Initialization... "<<endl;

        lowerGateIn  = findGate("lowerGateIn");
        lowerGateOut = findGate("lowerGateOut");

        Tsync = par("Tsync");

        // ---------------------------------------------------------------------------
        // ArpHost Module Parameters
        // parameter 'masterAddrOffset' is used to set the address of the master
        // set the master address, using the same IP, MAC address as ArpHost (see the *.ini file)
        // ---------------------------------------------------------------------------
        // RelayMaster[0] address: 2000; RelayMaster[1] address: 2001 ...
        if (hasPar("masterAddrOffset"))
            myAddress = (findHost() -> getIndex()) + (int)par("masterAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
        else
            error("No parameter masterAddrOffset is found");

        ev << "RelayMaster: my address is "<< myAddress <<endl;

        // Initialize the pointer to the clock module */
        pClock = (PCOClock *)getParentModule() -> getParentModule() -> getSubmodule("clock");
        if (pClock == NULL)
        {
            error("RelayMaster: No clock module is found in the network. ");
        }

  }

void RelayMaster::handleMessage(cMessage* msg)
{
    int whichGate;

    if (msg -> isSelfMessage())
    {
        error("RelayMaster: there should be no self message in relay master.\n");
        delete msg;
    }

    // no self-message,check where it comes from
    whichGate = msg -> getArrivalGateId();

    if (whichGate == lowerGateIn)
    {
        EV << "RelayMaster: receives a packet"<<endl;

        if (dynamic_cast<Packet *>(msg) != NULL)
        {
            EV << "RelayMaster: this is a Packet packet, RelayMaster now is processing it " <<endl;
            Packet *pck = static_cast<Packet *>(msg);
            if((pck -> getSource() != myAddress) &
                    ((pck -> getDestination() == myAddress) | (pck -> getDestination() == PACKET_BROADCAST_ADDR)))
             {
                EV << "RelayMaster: the packet is for me, process it \n";
                handleSlaveMessage(pck);   // handelSlaveMessage() does not delete msg
             }
             else
             {
                 EV << "RelayMaster: the packet is not for me, ignore it, do nothing \n";
             }

             delete msg;
        }
        else
        {
            EV << "RelayMaster: this is not a packet, send it up to higher layer" <<endl;
            send(msg, "upperGateOut");
        }
    }

    else if (whichGate == -1)
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

void RelayMaster::handleSelfMessage(cMessage *msg)
{
    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);
    pck -> setByteLength(TIMESTAMP_BYTE);

    pck -> setSource(myAddress);
    pck -> setDestination(PACKET_BROADCAST_ADDR);   // todo: if this function will be used in the
                                                    // future, please modify the destination address

    // pck->setData(SIMTIME_DBL(simTime()));

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setSrcAddr( LAddress::L3Type(myAddress));
    pck->setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "RelayMaster: broadcasts SYNC packet" << endl;
    send(pck,"lowerGateOut");

    scheduleAt(simTime() + Tsync, new cMessage("MStimer"));   // schedule the next time-synchronisation.
}

void RelayMaster::handleSlaveMessage(Packet *msg)
{
    mySlaveAddress = msg -> getSource();

    switch (msg -> getPacketType())
    {
        case REG:
        {
            if (mySlaveAddress == 1000)
            {
                ev << "RelayMaster: the received REGISTER packet is from the master, NOT for me, ignore it. \n";
                break;
            }
            else if ((mySlaveAddress >= 2000) & (mySlaveAddress < 3000))
            {
                ev << "RelayMaster: Register packet from relay, process it. \n";

                Packet *rplPkt= static_cast<Packet *>((Packet *)msg -> dup());
                rplPkt -> setName("REPLY_REGISGER");
                rplPkt -> setByteLength(0);
                rplPkt -> setPacketType(REGREP);

                rplPkt -> setSource(myAddress);
                rplPkt -> setDestination(((Packet *)msg) -> getSource());

                rplPkt -> setSrcAddr(LAddress::L3Type(rplPkt -> getSource()));
                rplPkt -> setDestAddr(LAddress::L3Type(rplPkt -> getDestination()));

                NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt -> getDestination()));
                send(rplPkt, "lowerGateOut");

                ev << "RelayMaster: REPLY_REGISGER packet is transmitted to relay. \n";
                break;
            }
            else if (mySlaveAddress >= 3000)
            {
                ev << "RelayMaster: Register packet from slave, process it. \n";
                Packet *rplPkt= static_cast<Packet *>((Packet *)msg -> dup());
                rplPkt -> setName("REPLY_REGISGER");
                rplPkt -> setByteLength(0);
                rplPkt -> setPacketType(REGREP);

                rplPkt -> setSource(myAddress);
                rplPkt -> setDestination(((Packet *)msg) -> getSource());

                rplPkt -> setSrcAddr(LAddress::L3Type(rplPkt -> getSource()));
                rplPkt -> setDestAddr(LAddress::L3Type(rplPkt -> getDestination()));

                NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt -> getDestination()));
                send(rplPkt, "lowerGateOut");

                ev << "RelayMaster: REPLY_REGISGER packet is transmitted to slave. \n";
                break;
            }
            else
            {
                ev << "RelayMaster: the received REGISTER packet is invalid, ignore it. \n";
                break;
            }
        }

        case REGREP:
        {
            if (mySlaveAddress == 1000)
            {
                ev << "RelayMaster: Received register_reply packet is from master, RelayMaster ignore it. \n";
                break;
            }
            else if ((mySlaveAddress >= 2000) & (mySlaveAddress < 3000))
            {
                ev << "RelayMaster: Received register_reply packet is from relay, RelayMaster ignore it. \n";
                break;
            }
            else if (mySlaveAddress >= 3000)
            {
                ev << "RelayMaster: Received register_reply packet is from slave, RelayMaster ignore it. \n";
                break;
            }
        }

        case SYNC:
        {
            ev << "RelayMaster: receives a SYNC packet, ignore it. \n";
            break;
        }

        default:
        {
            error("RelayMaster: receives unknown message, report warning and ignore. \n");
            break;
        }
    }
}

void RelayMaster::finish()
{

}

cModule *RelayMaster::findHost(void)
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

void RelayMaster::startNextHopSync()
{
    Enter_Method_Silent(); // see simuutil.h for detail

    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);
    pck -> setByteLength(TIMESTAMP_BYTE);

    pck -> setSource(myAddress);
    pck -> setDestination(PACKET_BROADCAST_ADDR);   // todo: if this function will be used in the
                                                    // future, please modify the destination address

    // pck->setData(SIMTIME_DBL(simTime()));

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr( LAddress::L3Type(myAddress));
    pck -> setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "RelayMaster: broadcasts SYNC packet" << endl;
    send(pck, "lowerGateOut");
}

