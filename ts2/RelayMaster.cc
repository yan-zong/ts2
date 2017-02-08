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

#include "RelayMaster.h"
#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "distrib.h"

Define_Module(RelayMaster);

void RelayMaster::initialize()
{
        ev << "Relay Master Initialisation "<<endl;

        lowerGateIn  = findGate("lowerGateIn");
        lowerGateOut = findGate("lowerGateOut");

        // ****************************************************************************
        // Variable Initialisation.
        // ****************************************************************************
        Tsync = par("Tsync");
        name = "master";
        nbReceivedDelayRequests = 0;
        nbSentSyncs = 0;
        nbSentDelayResponses = 0;

        // ****************************************************************************
        // ArpHost Module Parameters
        // parameter 'masterAddrOffset' is used to set the address of the master
        // set the master address, using the same IP, MAC address as ArpHost (see the *.ini file)
        // ****************************************************************************
        // RelayMaster[0] address: 2000; RelayMaster[1] address: 2001 ...
        if (hasPar("masterAddrOffset"))
            myAddress = (findHost()->getIndex()) + (int)par("masterAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
        else
            error("No parameter masterAddrOffset is found");

        ev << "Relay Master address is "<< myAddress <<endl;

        // Initialise the pointer to the clock module
        pClock = (Clock2 *)getParentModule()->getParentModule()->getSubmodule("clock");
        if (pClock == NULL)
        {
            error("No clock module is found in the module");
        }

        // find relay node, and index
        cModule *RelayModule = findHost()->getParentModule();
        ev<<"Relay Master: RelayModule: findHost()->getParentModule returns: "<< RelayModule->getName() <<endl;
        // Relay Slave: findHost()->getParentModule returns: TSieee802154SM

        RelayModule = RelayModule->getSubmodule("smnode", (findHost()->getIndex()));
        RelayIndex = findHost()->getIndex();
        ev<<"Relay Master: RelayModule Index is "<< RelayIndex <<endl;

        // Schedule the time-synchronisation of relay
        // ToDo: add a random value function to improve the efficiency of packet exchange
        RandomTime = uniform(0,1,0);

        if (hasPar("RandomValue"))
        {
            ScheduleRandomTime = RandomTime * ((double)par("RandomValue"));
        }
        else
        {
            error("No Parameter RandomValue is found in the *.ini file");
        }

        scheduleAt(simTime() + Tsync + ScheduleRandomTime, new cMessage("MStimer"));
        ev<<"Relay Master: debug: simTime() = "<< simTime() <<endl;
        ev<<"Relay Master: debug: Tsync = "<< Tsync <<endl;
        ev<<"Relay Master: debug: simTime() + Tsync = "<< simTime() + Tsync <<endl;
        ev<<"Relay Master: uniform(0,1,0) = "<< uniform(0,1,0) <<endl;

        // ****************************************************************************
        // Register slave
        // ****************************************************************************
        PtpPkt * temp = new PtpPkt("REGISTER");
        temp->setPtpType(REG);

        // use the host modules findHost() as a application address
        temp->setDestination(PTP_BROADCAST_ADDR);
        temp->setSource(myAddress);

        // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
        // L3: Network Layer
        temp->setDestAddr(LAddress::L3BROADCAST);
        temp->setSrcAddr( LAddress::L3Type(myAddress));
        temp->setByteLength(0);

        // set the control info to tell the network layer (layer 3) address
        NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

        EV << "Relay Master broadcasts REGISTER packet" << endl;
        send(temp,"lowerGateOut");
        ev << "debug: packet->getdestination() = " << temp -> getDestination() <<".\n";
  }

void RelayMaster::handleMessage(cMessage* msg)
{
    int whichGate;

    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
        return;
    }

    // no self-message,check where it comes from
    whichGate = msg -> getArrivalGateId();
    if (whichGate == lowerGateIn)
    {
        // check PtpPkt type
        EV << "Relay Master receives a packet"<<endl;

        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {
            EV << "this ia a PtpPkt packet, Relay Master now is processing it " <<endl;
            PtpPkt *pck = static_cast<PtpPkt *>(msg);
            if((pck->getSource() != myAddress) &
                    ((pck->getDestination() == myAddress) | (pck->getDestination() == PTP_BROADCAST_ADDR)))
             {
                EV << "the PtpPkt packet is for me, process it\n";
                 handleSlaveMessage(pck);   // handelSlaveMessage() does not delete msg
             }
             else
             {
                 EV << "the PtpPck packet is not for me, ignore it, do nothing\n";
             }
             delete msg;
        }
        else
        {
            EV << "This is not a PtpPkt packet, send it up to higher layer" <<endl;
            send(msg,"upperGateOut");
        }
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

void RelayMaster::handleSelfMessage(cMessage *msg)
{
    EV << "Relay Master Timer fired New 'SYNC' Packet\n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck -> setPtpType(SYNC);
    pck -> setByteLength(SYNC_BYTE);

    pck -> setTimestamp(simTime());

    pck -> setSource(myAddress);
    pck -> setDestination(PTP_BROADCAST_ADDR);

    // ToDo: the time-stamping of the master in the relay node, need to be time-stamped by the
    // inaccurate clock, i.e., the clock module of the relay node
    pck -> setData(SIMTIME_DBL(simTime()));
    pck -> setTsTx(SIMTIME_DBL(simTime())); // set transmission time stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr( LAddress::L3Type(myAddress));
    pck -> setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "Relay Master broadcasts SYNC packet" << endl;
    send(pck,"lowerGateOut");

    nbSentSyncs = nbSentSyncs + 1;  // count the total number of the sent SYNC packet

    scheduleAt(simTime()+Tsync, new cMessage("MStimer"));   // schedule the next time-synchronisation.
}

void RelayMaster::handleSlaveMessage(PtpPkt *msg)
{
    mySlaveAddress = msg->getSource();
    ev << "Relay Master: mySlaveAddress = " << mySlaveAddress <<".\n";

    switch (msg->getPtpType()){
    case REG:
    {
        if (mySlaveAddress == 1000)
        {
            ev << "Relay Master: the received REGISTER packet is from the master, NOT for me, ignore it \n";
            break;
        }
        else if (mySlaveAddress == 2000 || mySlaveAddress > 2000)
        {
            ev << "Relay Master: the received REGISTER packet is from the Relay node, generate the REPLY_REGISTER packet\n";

            PtpPkt *rplPkt= static_cast<PtpPkt *>((PtpPkt *)msg->dup());
            rplPkt -> setName("REPLY_REGISGER");
            rplPkt -> setByteLength(0);
            rplPkt -> setPtpType(REGREPLY);

            rplPkt -> setSource(myAddress);
            rplPkt -> setDestination(mySlaveAddress);

            // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
            rplPkt -> setSrcAddr(LAddress::L3Type(myAddress));
            rplPkt -> setDestAddr(LAddress::L3Type(mySlaveAddress));

            // set the control info to tell the network layer (layer 3) address
            NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(mySlaveAddress));

            EV << "Relay Master transmits REPLY_REGISGE Rpacket" << endl;
            send(rplPkt, "lowerGateOut");
            break;
        }
        else
        {
            ev << "the received REGISTER packet is invalid, ignore it \n";
            break;
        }
    }
    case REGREPLY:
    {
        ev << "Relay Master receive REGISTER_REPLY packet (PtpType = REGREPLY), ignore\n";
        break;
    }
    case SYNC:
    {
        ev << "Relay Master receives a SYNC packet, ignore it\n";
        break;
    }
    case DREQ:
    {
        if (mySlaveAddress == 1000)
        {
            ev << "Relay Master: the received DREQ packet is from the master, NOT for me, ignore it \n";
            break;
        }
        else if (mySlaveAddress == 2000 || mySlaveAddress > 2000)
        {
            ev << "Relay Master: the received DREQ packet is from the Relay node, generate the DRES packet\n";

            PtpPkt *pck = new PtpPkt("DRES");
            pck -> setByteLength(DRES_BYTE);

            pck -> setDestination(mySlaveAddress);
            pck -> setSource(myAddress);

            pck->setPtpType(DRES);
            pck->setData(SIMTIME_DBL(simTime()));

            // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
            pck->setSrcAddr(LAddress::L3Type(myAddress));
            pck->setDestAddr(LAddress::L3Type(mySlaveAddress));

            // set the control info to tell the network layer (layer 3) address
            NetwControlInfo::setControlInfo(pck, LAddress::L3Type(mySlaveAddress));

            EV << "Relay Master transmits DRES packet" << endl;
            send(pck,"lowerGateOut");

            nbReceivedDelayRequests = nbReceivedDelayRequests + 1;  // count the number of the received delay request packet.

            nbSentDelayResponses = nbSentDelayResponses + 1;  // count the number of the received delay request packet.

            break;
        }
        else
        {
            ev << "Relay Master: the received DREQ packet is NOT for me, ignore it \n";
            break;
        }
    }
    case DRES:
    {
        ev << "Relay Master receives DRES packet, ignore it\n";
        break;
    }
    default:
    {
        error("Relay Master receives unknown message, report warning and ignore");
        break;
    }
    }
}

void RelayMaster::finish()
{
    EV << "nbReceivedDelayRequests = " << nbReceivedDelayRequests << endl;
    EV << "nbSentSyncs = " << nbSentSyncs << endl;
    EV << "nbSentDelayResponses = " << nbSentDelayResponses << endl;

    recordScalar("nbReceivedDelayRequests", nbReceivedDelayRequests);
    recordScalar("nbSentSyncs", nbSentSyncs);
    recordScalar("nbSentDelayResponses", nbSentDelayResponses);
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

void RelayMaster::startSync()
{
    Enter_Method_Silent(); // see simuutil.h for detail

    EV << "Relay Master Timer fired New 'SYNC' Packet for slave of the second-hop\n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck -> setPtpType(SYNC);
    pck -> setByteLength(SYNC_BYTE);

    pck -> setTimestamp(simTime());

    pck -> setSource(myAddress);
    pck -> setDestination(PTP_BROADCAST_ADDR);

    pck -> setData(SIMTIME_DBL(simTime()));
    pck -> setTsTx(SIMTIME_DBL(simTime())); // set transmission time stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr( LAddress::L3Type(myAddress));
    pck -> setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "Relay Master broadcasts SYNC packet to Slave" << endl;
    send(pck,"lowerGateOut");

    nbSentSyncs = nbSentSyncs + 1;  // count the total number of the sent SYNC packet
}

