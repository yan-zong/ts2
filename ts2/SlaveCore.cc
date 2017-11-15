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

#include "SlaveCore.h"

#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "Constant.h"

Define_Module(SlaveCore);

void SlaveCore::initialize()
{
    upperGateIn  = findGate("upperGateIn");
    upperGateOut = findGate("upperGateOut");
    lowerGateIn  = findGate("lowerGateIn");
    lowerGateOut = findGate("lowerGateOut");
    inclock = findGate ("inclock");
    // outclock = findGate ("outclock");
    // inevent = findGate ("inevent");

    if(ev.isGUI())
	{
	    updateDisplay();
	}

	/* Intilaize the pointer to the clock module */
	pClock = (PCOClock *)getParentModule() -> getSubmodule("clock");
	if (pClock == NULL)
	    error("Slave: No clock module is found");

	/* Initialize the addresses of slave & master */
	if (hasPar("slaveAddrOffset"))
	    address = findHost() -> getIndex() + (int)par("slaveAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
	else
	    address = findHost() -> getIndex();
	ev<<"SlaveCore: my address is "<< address << endl;

	/* Find the master node and its address. */
	cModule *masterModule = findHost() -> getParentModule();
	ev<<"findHost() -> getParentModule returns: "<< masterModule -> getName() <<endl;
	masterModule = masterModule -> getSubmodule("mnode");

	/* Find the relay node and its address. */
	cModule *relayModule = findHost() -> getParentModule();
	ev<<"findHost() -> getParentModule returns: "<< relayModule -> getName() <<endl;
	relayModule = relayModule -> getSubmodule("rnode", (findHost() -> getIndex()));

    if ((masterModule == NULL) & (relayModule == NULL))
    {
        error("No Sink node or RelayMaster are found");
    }

	PtpPkt * temp = new PtpPkt("REGISTER");
    temp->setSource(address);
    temp->setDestination(PTP_BROADCAST_ADDR);
    temp->setPtpType(REG);
    temp->setByteLength(0);

    temp->setDestAddr(LAddress::L3BROADCAST);
    temp->setSrcAddr(address);

    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

    send(temp,"lowerGateOut");
	ev << "SlaveCore: Slave Node " << getId() << " : Initialisation finished. Send Register packet\n";
}

void SlaveCore::handleMessage(cMessage *msg)
{
    ev << "SlaveCore: handleMessage invoked\n";

    if(msg->isSelfMessage())
    {
        error("error in SlaveCore module, there should be no self message in slave core.\n");
        delete msg;

        // handleSelfMessage(msg);
    }

    if (msg -> arrivedOn("inclock"))
	{
	    EV << "SlaveCore: Slave receives a SYNC packet from clock module, delete it and DON't re-generate a full SYNC packet due to the it is slave node.\n";
	    delete msg;
	    EV << "SlaveCore: the received SYNC from clock module is deleted.\n";

	    // scheduleAt(simTime(), new cMessage("OffsetTimer"));
	    // handleClockMessage(msg);
	}

    if (msg -> arrivedOn("lowerGateIn"))  // data packet from lower layer
	{   // check PtpPkt type
        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {
            EV << "SlaveCore receives a PtpPkt packet. ";
             if(((PtpPkt*)msg) -> getSource() != address &
                     (((PtpPkt*)msg) -> getDestination() == address |
                             ((PtpPkt*)msg) -> getDestination() == PTP_BROADCAST_ADDR))
             {
                 EV << "the packet is for me, process it\n";
                 handleMasterMessage(msg);
             }
             else
             {
                 EV << "the packet is not for me, ignore it\n";
             }

             delete msg;

        }
        else
        {
            EV << " this packtet is not a PtpPkt packet, send it up to higher layer"<<endl;
            send(((PtpPkt *)msg)->dup(),"upperGateOut");
         }
	}

    if (msg -> arrivedOn("upperGateIn"))   // data packet from upper layer
    {
         EV << "receive a PtpPkt packet from higher layer"<<endl;
         send(msg, "lowerGateOut");
    }

    if (msg -> arrivedOn("inevent"))
    {
        handleEventMessage(msg);
        delete msg;
    }

	if(ev.isGUI())
	{
	    updateDisplay();
	}
}

void SlaveCore::handleSelfMessage(cMessage *msg)
{
    PtpPkt *pck = new PtpPkt("SYNC");
    pck -> setPtpType(SYNC);
    pck -> setByteLength(44);

    // pck -> setTimestamp(simTime());

    pck -> setSource(address);
    pck -> setDestination(PTP_BROADCAST_ADDR);

    // pck -> setData(SIMTIME_DBL(simTime()));
    // pck -> setTsTx(SIMTIME_DBL(simTime())); // set transmission time stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr(LAddress::L3Type(pck -> getSource()));
    pck -> setDestAddr(LAddress::L3Type(pck -> getDestination()));

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3Type(pck -> getDestination()));

    send((cMessage *)pck,"lowerGateOut");
    EV << "Slave broadcasts SYNC packet" << endl;
}

void SlaveCore::handleOtherPacket(cMessage *msg)
{
}

void SlaveCore::handleEventMessage(cMessage *msg)
{
	if(((Event *)msg)->getEventType()==CICLICO)
	{
		Packet *pck = new Packet("CICLICO");
		pck->setPckType(OTHER);
		pck->setSource(address);
		pck->setDestination(((Event *)msg)->getDest());
		pck->setByteLength(((Event *)msg)->getPckLength());

		for(int i=0; i<((Event *)msg)->getPckNumber()-1; i++)
		{
		    send((cMessage *)pck->dup(),"lowerGateOut");
		}
		send(pck,"lowerGateOut");
	}
}


void SlaveCore::handleClockMessage(cMessage *msg)
{

}

void SlaveCore::handleMasterMessage(cMessage *msg)
{
    switch(((PtpPkt *)msg)->getPtpType())
    {
		case SYNC:
		{

            if ((((PtpPkt *)msg) -> getSource())  == 1000)
            {
                ev << "Relay Slave receives SYNC packet from master node, process it\n";

                // get the measurement offset based on the reception of SYNC from master,
                // and no need to use the 'AddressOffset.#
                ev << "Slave: get the offset and skew...\n";
                EstimatedOffset = pClock -> getMeasurementOffset(4, 0);
                EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);

                ev << "Slave: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "Slave: Done.\n";

                break;

            }
            else if ( ((((PtpPkt *)msg) -> getSource()) >= 2000) || ((((PtpPkt *)msg) -> getSource()) < 3000) )
            {
                ev << "Slave receives SYNC packet from relay node, process it\n";

                ev << "Slave: get the offset and skew...\n";
                EstimatedOffset = pClock -> getMeasurementOffset(4, 0);
                EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);

                ev << "Slave: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "Slave: Done.\n";

                break;

            }
            else
            {
                ev << "Slave: the received SYNC packet is invalid, ignore it \n";
                break;
            }
		}

		case REG:
		{
		    ev<<"Register package, slave node does nothing. returns\n";
			break;
		}

        case REGRELAY:
        {
            ev << "receive REGISTER package, slave node does nothing. returns\n";
            break;
        }

        case REGREPLY:
        {
            if ((((PtpPkt *)msg) -> getSource()) == 1000)
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from master node, process it\n";
                myMasterAddress == (((PtpPkt *)msg) -> getSource());
                ev<<"my master's address is updated to "<< myMasterAddress << " according to the the REGPREPLY packet)\n";
                break;
            }
            else if ( ((((PtpPkt *)msg) -> getSource()) >= 2000) || ((((PtpPkt *)msg) -> getSource()) < 3000) )
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from Relay node, process it\n";
                myRelayAddress == (((PtpPkt *)msg) -> getSource());
                ev<<"my relay's address is updated to "<< myRelayAddress << " according to the the REGPREPLY packet)\n";
                break;
            }
            else
            {
                ev << "Slave: the received REGREPLY packet is invalid, ignore it \n";
                break;
            }
        }
		case DREQ:
		{
		    ev << "Invalid master message DREQ. Ignored\n";
		    break;
		}
	}
}

void SlaveCore::recordResult()
{

}

void SlaveCore::finish()
{
}

void SlaveCore::updateDisplay()
{
//	char buf[100];
//	sprintf(buf, "dms [ms]: %3.2f \ndsm [ms]: %3.2f \ndpr [ms]: %3.2f \noffset [ms]: %3.2f\n ",
//		dms*1000,dsm*1000,dprop*1000,offset*1000);
//	getDisplayString().setTagArg("t",0,buf);
}

cModule *SlaveCore::findHost(void)
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
