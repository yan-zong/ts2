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
	    error("SlaveCore: No clock module is found");

    // ---------------------------------------------------------------------------
    // ArpHost module parameters
    // parameter 'slaveAddrOffset' is used to set the address of the slave
    // set the slave address, using the same IP, MAC address as ArpHost (see the *.ini file)
    // ---------------------------------------------------------------------------
    // Slave[0] address: 3000; Slave[1] address: 3001 ...
	if (hasPar("slaveAddrOffset"))
	    address = findHost() -> getIndex() + (int)par("slaveAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
	else
	    address = findHost() -> getIndex();
	ev<<"SlaveCore: my address is "<< address << endl;

	/* Find the master node and its address. */
	cModule *masterModule = findHost() -> getParentModule();
	ev<<"SlaveCore: findHost() -> getParentModule returns: "<< masterModule -> getName() <<endl;
	masterModule = masterModule -> getSubmodule("mnode");

	/* Find the relay node and its address. */
	cModule *relayModule = findHost() -> getParentModule();
	ev<<"SlaveCore: findHost() -> getParentModule returns: "<< relayModule -> getName() <<endl;
	relayModule = relayModule -> getSubmodule("rnode", (findHost() -> getIndex()));

    if ((masterModule == NULL) & (relayModule == NULL))
    {
        error("SlaveCore: No master node or relay are found");
    }

	Packet * temp = new Packet("REGISTER");
    temp->setSource(address);
    temp->setDestination(PACKET_BROADCAST_ADDR);
    temp->setPacketType(REG);
    temp->setByteLength(0);

    temp->setDestAddr(LAddress::L3BROADCAST);
    temp->setSrcAddr(address);

    // set the control info to tell the network layer the layer 3 address
    NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

    send(temp,"lowerGateOut");
	ev << "SlaveCore: Slave Node " << getId() << " : Initialization finished. Send Register packet. \n";
}

void SlaveCore::handleMessage(cMessage *msg)
{
    ev << "SlaveCore: handleMessage invoked. \n";

    if(msg -> isSelfMessage())
    {
        error("SlaveCore: there should not be selfMessage in the slave core module");

        // handleSelfMessage(msg);
        return;
    }

    if (msg -> arrivedOn("inclock"))
	{
	    EV << "SlaveCore: receives a SYNC packet from clock module, delete it and DON't re-generate a full SYNC packet due to it is slave node.\n";
	    delete msg;

	    // scheduleAt(simTime(), new cMessage("FireTimer"));
	}

    if (msg -> arrivedOn("lowerGateIn"))  // data packet from lower layer
	{   // check PtpPkt type
        if (dynamic_cast<Packet *>(msg) != NULL)
        {
            EV << "SlaveCore: receives a packet. ";
             if((((Packet*)msg) -> getSource() != address) &
                     (((((Packet*)msg) -> getDestination() == address) |
                             (((Packet*)msg) -> getDestination() == PACKET_BROADCAST_ADDR))))
             {
                 EV << "the Packet is for me, process it. \n";
                 handleMasterMessage(msg);
             }
             else
             {
                 EV << "the Packet is not for me, ignore it. \n";
             }

             delete msg;
        }
        else
        {
            EV << " this Packet is not a Packet, send it up to higher layer"<<endl;
            send(((Packet *)msg) -> dup(),"upperGateOut");
         }
	}

    if (msg -> arrivedOn("upperGateIn"))   // data packet from upper layer
    {
         EV << "receive a Packet from higher layer"<<endl;
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
    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);
    pck -> setByteLength(TIMESTAMP_BYTE);

    pck -> setSource(address);
    pck -> setDestination(PACKET_BROADCAST_ADDR);

    // pck -> setData(SIMTIME_DBL(simTime()));

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr(LAddress::L3Type(pck -> getSource()));
    pck -> setDestAddr(LAddress::L3Type(pck -> getDestination()));

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3Type(pck -> getDestination()));

    send((cMessage *)pck,"lowerGateOut");
    EV << "SlaveCore: broadcasts SYNC packet" << endl;
}

void SlaveCore::handleOtherPacket(cMessage *msg)
{

}

void SlaveCore::handleEventMessage(cMessage *msg)
{
	if(((Event *)msg) -> getEventType()==CICLICO)
	{
		Packet *pck = new Packet("CICLICO");
		pck -> setPacketType(OTHER);
		pck -> setSource(address);
		pck -> setDestination(((Event *)msg) -> getDest());
		pck -> setByteLength(((Event *)msg) -> getPckLength());

		for(int i=0; i<((Event *)msg) -> getPckNumber()-1; i++)
		{
		    send((cMessage *)pck -> dup(),"lowerGateOut");
		}
		send(pck,"lowerGateOut");
	}
}

void SlaveCore::handleMasterMessage(cMessage *msg)
{
    int AddressOffset;

    switch(((Packet *)msg) -> getPacketType())
    {
        case REG:
        {
            ev<<"SlaveCore: receive Register package, slave node does nothing. \n";
            break;
        }

        case REGREP:
        {
            if ((((Packet *)msg) -> getSource()) == 1000)
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from master node, process it. \n";
                myMasterAddress == (((Packet *)msg) -> getSource());
                ev<<"SlaveCore: my master's address is updated to "<< myMasterAddress << " according to the the REGPREPLY packet)\n";
                break;
            }
            else if ( ((((Packet *)msg) -> getSource()) >= 2000) & ((((Packet *)msg) -> getSource()) < 3000))
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from Relay node, process it. \n";
                myRelayAddress == (((Packet *)msg) -> getSource());
                ev<<"SlaveCore: my relay's address is updated to "<< myRelayAddress << " according to the the REGPREPLY packet)\n";
                break;
            }
            else if ((((Packet *)msg) -> getSource()) >= 3000)
             {
                 ev << "Relay Slave receives REGISTER_REPLY packet from slave node,ignore it. \n";
                 break;
             }
            else
            {
                ev << "SlaveCore: the received REGREPLY packet is invalid, ignore it \n";
                break;
            }
        }

		case SYNC:
		{

            if ((((Packet *)msg) -> getSource())  == 1000)
            {
                ev << "SlaveCore: receives SYNC packet from master node, process it\n";

                // get the measurement offset based on the reception of SYNC from master,
                // and no need to use the 'AddressOffset'
                ev << "SlaveCore: get the offset and skew...\n";

                pClock -> setReceivedSYNCTime((((Packet *)msg) -> getTsRx()));

                ev << "SlaveCore: timestamp is "<< (((Packet *)msg) -> getTsRx()) << endl;

                EstimatedOffset = pClock -> getMeasurementOffset(5, 0);
                EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);

                ev << "SlaveCore: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "SlaveCore: Done.\n";

                break;

            }
            else if ( ((((Packet *)msg) -> getSource()) >= 2000) || ((((Packet *)msg) -> getSource()) < 3000) )
            {
                ev << "SlaveCore: receives SYNC packet from relay node, process it\n";

                ev << "SlaveCore: get the offset and skew...\n";
                AddressOffset = (((Packet *)msg) -> getSource()) - (2000 - 1);

                pClock -> setReceivedSYNCTime((((Packet *)msg) -> getTsRx()));

                ev << "SlaveCore: timestamp is "<< (((Packet *)msg) -> getTsRx()) << endl;

                EstimatedOffset = pClock -> getMeasurementOffset(4, AddressOffset);
                EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);

                ev << "SlaveCore: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "SlaveCore: Done.\n";

                break;

            }
            else if ((((Packet *)msg) -> getSource()) >= 3000)
            {
                ev << "SlaveCore: receives SYNC packet from slave node, ignore it\n";
                break;

            }
            else
            {
                ev << "SlaveCore: the received SYNC packet is invalid, ignore it \n";
                break;
            }
		}

        case DRES:
        {
            ev << "SlaveCore: Invalid message DRES. Ignored\n";
            break;
        }

        default:
        {
            error("SlaveCore: receives unknown message, report warning and ignore. \n");
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
