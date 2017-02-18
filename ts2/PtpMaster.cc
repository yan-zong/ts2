//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  29 Jan 2014
// * Author:      Xuweu Dai  (x.dai at ieee.org)
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

#include "PtpMaster.h"

#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"

Define_Module(PtpMaster);

void PtpMaster::initialize()
{
    ev<<"DXw: Initialize PtpMaaster"<<endl;
        upperGateIn  = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
        lowerGateIn  = findGate("lowerGateIn");
        lowerGateOut = findGate("lowerGateOut");
 //        upperControlIn  = findGate("upperControlIn");
//        upperControlOut = findGate("upperControlOut");
//        lowerControlIn  = findGate("lowerControlIn");
//        lowerControlOut = findGate("lowerControlOut");

        // ---------------------------------------------------------------------------
        // Inizializzazione variabili.
        // ---------------------------------------------------------------------------
        Tsync = par("Tsync");
        name = "master";

        // set the master address, using the same IP, MAC address as ArpHost.
        //  see the omnetpp.ini,
        // ################ ArpHost module parameters ####################
        //    *.mnode*.arp.offset = 100
        //    *.mnode*.ptpcore.masterAddrOffset=100
        if (hasPar("masterAddrOffset"))
            address = findHost()->getIndex()+(int)par("masterAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
        else
            address = findHost()->getIndex();

        scheduleAt(simTime()+Tsync, new cMessage("MStimer"));
        // ---------------------------------------------------------------------------
        // Register node
        // ---------------------------------------------------------------------------
        PtpPkt * temp = new PtpPkt("REGISTER");
        // we use the host modules findHost() as a appl address
        temp->setDestination(PTP_BROADCAST_ADDR); //PTP_BROADCAST_ADDR = -1
        temp->setSource(address);
        temp->setPtpType(REG);

        temp->setDestAddr(LAddress::L3BROADCAST);
        temp->setSrcAddr( LAddress::L3Type(address));
        temp->setByteLength(0);
        // set the control info to tell the network layer the layer 3
        // address;
        NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

        EV << "PtpCore broadcasts REGISTER packet!" << endl;
        send(temp,"lowerGateOut");
  }


/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
 **/
void PtpMaster::handleMessage(cMessage* msg)
{
    int whichGate;

    if (msg->isSelfMessage()){
        handleSelfMessage(msg);
        return;
    }
    // not self-message,check where it comes frome
    whichGate=msg->getArrivalGateId();
    if(whichGate==upperGateIn) {
        send(msg,"lowerGateOut");
    } else if(whichGate==lowerGateIn) {
        // check PtpPkt type
        EV<<"Received a packet"<<endl;

        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {  EV<<"This ia a PtpPkt packet, PtpMaster now is processing it "<<endl;
           PtpPkt *pck= static_cast<PtpPkt *>(msg);
           if(pck->getSource()!=address &
               (pck->getDestination()==address | pck->getDestination()==PTP_BROADCAST_ADDR))
                      //PTP_BROADCAST_ADDR = -1
             {   EV<<"the PtpPkt is for me, process it\n";
                 handleSlaveMessage(pck); // handelSlaveMessage() does not delete msg
             }
             else
                 EV<<"the PtpPck is not for me, ignore it. Do nothing\n";

             delete msg;
        }
        else
        {   EV<<"NOt a PtpPkt packet, semd it up to higher layer"<<endl;
             send(msg,"upperGateOut");
         }
    } else if(whichGate==-1) {
        /* Classes extending this class may not use all the gates, f.e.
         * BaseApplLayer has no upper gates. In this case all upper gate-
         * handles are initialized to -1. When getArrivalGateId() equals -1,
         * it would be wrong to forward the message to one of these gates,
         * as they actually don't exist, so raise an error instead.
         */
        opp_error("No self message and no gateID?? Check configuration.");
    } else {
        /* msg->getArrivalGateId() should be valid, but it isn't recognized
         * here. This could signal the case that this class is extended
         * with extra gates, but handleMessage() isn't overridden to
         * check for the new gate(s).
         */
        opp_error("Unknown gateID?? Check configuration or override handleMessage().");
    }
}

/*********************************************************************************
 Private member function
**********************************************************************************/
void PtpMaster::handleSelfMessage(cMessage *msg){

 //   LAddress::L2Type macAddr;
 //   LAddress::L3Type netwAddr;

    EV <<"Tysnc Timer fired ... new SYNC PtpPkt\n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    // pck->setClockType(TIME_REQ);
    pck->setByteLength(40); // SYNC_BYTE = 40
    pck->setTimestamp(simTime());
    pck->setSource(address);
    pck->setDestination(-1);
    pck->setData(SIMTIME_DBL(simTime()));
    pck->setTsTx(SIMTIME_DBL(simTime())); // set transmission timie stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setSrcAddr( LAddress::L3Type(address));
    pck->setDestAddr(LAddress::L3BROADCAST);
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );
    EV << "PtpCore broadcasts SYNC packet!" << endl;
    send(pck,"lowerGateOut");
    scheduleAt(simTime()+Tsync, new cMessage("MStimer"));
}



void PtpMaster::handleSlaveMessage(PtpPkt *msg){

    switch (msg->getPtpType()){
        case SYNC:
                ev<<"PtpMaster::handleSlaveMessage() receives a SYNC packet, ignore.\n";
                break;
        case DRES:
            error("Invalid slave message");
            break;
        case DREQ:
        {  PtpPkt *pck = new PtpPkt("DRES");
            pck->setByteLength(50);  // DRES_BYTE = 50
            pck->setDestination(((PtpPkt *)msg)->getSource());
            pck->setSource(address);
            pck->setPtpType(DRES);
            pck->setData(SIMTIME_DBL(simTime()));

            pck->setPtpType(DRES);

            pck->setSrcAddr(LAddress::L3Type(address));
            pck->setDestAddr(LAddress::L3Type(pck->getDestination()));
            NetwControlInfo::setControlInfo(pck, LAddress::L3Type(pck->getDestination()));

            send(pck,"lowerGateOut");
            //Comments by XDai: do we need to delete msg? msg will be deleted by handleMessage()
            break;
        }
        case REG:
        {   // Receive Register packet from other nodes
            // Nor reply
             PtpPkt *rplPkt= static_cast<PtpPkt *>((PtpPkt *)msg->dup());
             rplPkt->setName("REPLY_REGISGER");
             rplPkt->setByteLength(0);
             rplPkt->setPtpType(REGREPLY);
             rplPkt->setSource(address);
             rplPkt->setDestination(((PtpPkt *)msg)->getSource());

             rplPkt->setSrcAddr(LAddress::L3Type(rplPkt->getSource()));
             rplPkt->setDestAddr(LAddress::L3Type(rplPkt->getDestination()));

             // for debug use broadcast
             // NetwControlInfo::setControlInfo(rplpck, LAddress::L3Type(rplPkt->getDestination()));
             NetwControlInfo::setControlInfo(rplPkt, LAddress::L3Type(rplPkt->getDestination()));
             send(rplPkt, "lowerGateOut");
             break;
        }
        case REGRELAYMASTER:
        {
            ev << " Register packet from relay node, ignore\n";
            break;
        }
        case REGREPLY:
        {
            ev<<"Received register_reply packet (PtpType=REGREPLY), ignore\n";
            break;
        }
        default:
        {
            ev<<"unknown message. Report warning and ignore\n";
            break;
        }
    }
}

void PtpMaster::finish(){
}

cModule *PtpMaster::findHost(void)
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

