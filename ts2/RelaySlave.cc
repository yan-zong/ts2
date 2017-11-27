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

#include "RelaySlave.h"
#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "ApplPkt_m.h"

Define_Module(RelaySlave);

void RelaySlave::initialize()
{
    ev << "RelaySlave: Initialization... \n";

	if(ev.isGUI())
	{
	    updateDisplay();
	}

	/* Initialize the pointer to the clock module */
	pClock = (PCOClock *)getParentModule() -> getParentModule() -> getSubmodule("clock");
	if (pClock == NULL)
	    error("RelaySlave: No clock module is found in the module");

	// ---------------------------------------------------------------------------
    // ArpHost module parameters
    // parameter 'slaveAddrOffset' is used to set the address of the slave
    // set the slave address, using the same IP, MAC address as ArpHost (see the *.ini file)
    // ---------------------------------------------------------------------------
	// RelaySlave[0] address: 2000; RelaySlave[1] address: 2001 ...
	if (hasPar("slaveAddrOffset"))
	    myAddress = (findHost() -> getIndex()) + (int)par("slaveAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
	else
	    error("RelaySlave: No parameter slaveAddrOffset is found");

    ev << "RelaySlave: my address is "<< myAddress <<endl;

	// Find the Relay Master, and Relay Master pointer
	pRelayMaster = (RelayMaster *)getParentModule() -> getSubmodule("RelayMaster");
	ev<<"RelaySlave: RelayMaster pointer is "<< pRelayMaster <<endl;
    if (pRelayMaster == NULL)
    {
        error("RelaySlave: could not find RelayMaster module in a Relay node (boundary clock node)");
    }

    Packet * temp = new Packet("REGISTER");
	temp -> setPacketType(REG);
	temp -> setByteLength(0);

    // use the host modules findHost() as a appl address
    temp -> setSource(myAddress);
    temp -> setDestination(PACKET_BROADCAST_ADDR); //PTP_BROADCAST_ADDR = -1

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    temp->setDestAddr(LAddress::L3BROADCAST);
    temp->setSrcAddr(myAddress);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

    send(temp,"out");
	ev << "RelaySlave: initialization finished, send REGISTER \n";
}

// msg is always deleted at the end of handlMessage(), but the temp(msg) is not deleted at the end of initialize()
void RelaySlave::handleMessage(cMessage *msg)
{
	ev << "RelaySlave: handleMessage invoked. \n";

    if (msg -> isSelfMessage())
    {
        handleSelfMessage(msg);

        return;
    }

	if (msg -> arrivedOn("inclock"))
	{
	    EV << "RelaySlave: receives a SYNC packet from clock module, delete it and re-generate a full SYNC packet \n";
	    delete msg;

	    scheduleAt(simTime(), new cMessage("FireTimer"));
	    // handleClockMessage(msg);
	}

	if (msg -> arrivedOn("in"))   // data packet from lower layer
	{
        if (dynamic_cast<Packet *>(msg) != NULL)
        {
            EV << "RelaySlave: receives a Packet, ";
            if((((Packet*)msg) -> getSource() != myAddress)  &
                     (((((Packet*)msg) -> getDestination() == myAddress)  |
                             (((Packet*)msg) -> getDestination() == PACKET_BROADCAST_ADDR))))
            {
                EV << "the Packet is for me, Relay Slave is processing it\n";
                handleMasterMessage(msg);
            }
            else
                EV << "the Packet is not for me, ignore it, do nothing\n";

            delete msg;
        }
        else
        {
            EV << "the packet is not for me, send it up to higher layer. \n"<<endl;
            send(msg, "upperGateOut");
         }
	}

    if (msg -> arrivedOn("upperGateIn"))   // data packet from upper layer
    {
         EV << "RelaySlave: receive a packet from higher layer. "<<endl;
         send(msg, "out");
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

void RelaySlave::handleSelfMessage(cMessage *msg)
{
    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);
    pck -> setByteLength(TIMESTAMP_BYTE);

    pck -> setSource(myAddress);
    pck -> setDestination(PACKET_BROADCAST_ADDR);

    // pck -> setData(SIMTIME_DBL(simTime()));

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck -> setSrcAddr( LAddress::L3Type(myAddress));
    pck -> setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "RelaySlave: broadcasts a SYNC packet. " << endl;
    send(pck,"out");

}

void RelaySlave::handleOtherPacket(cMessage *msg)
{
}

void RelaySlave::handleEventMessage(cMessage *msg)
{
	if(((Event *)msg)->getEventType()==CICLICO)
	{
		Packet *pck = new Packet("CICLICO");
		pck->setPacketType(OTHER);
		pck->setByteLength(((Event *)msg)->getPckLength());

		pck->setSource(myAddress);
		pck->setDestination(((Event *)msg)->getDest());

		for(int i=0; i<((Event *)msg)->getPckNumber()-1; i++)
		{
		    send((cMessage *)pck->dup(),"out");
		}
		send(pck,"out");
	}
}

void RelaySlave::handleMasterMessage(cMessage *msg)
{
    // myMasterAddress = (((Packet *)msg) -> getSource());

    switch(((Packet *)msg) -> getPacketType())
    {
        case REG:
        {
            if ((((Packet *)msg) -> getSource()) == 1000)
            {
                ev << "RelaySlave: receive REGISTER package from master, RelaySlave does nothing. \n";
                break;
            }
            else if (((((Packet *)msg) -> getSource()) >= 2000) & ((((Packet *)msg) -> getSource()) < 3000))
            {
                ev << "RelaySlave: receive REGISTER package from relay, RelaySlave does nothing. \n";
                break;
            }
            else if ((((Packet *)msg) -> getSource()) >= 3000)
            {
                ev << "RelaySlave: receive REGISTER package from slave, RelaySlave does nothing. \n";
                break;
            }
        }

        case REGREP:
        {
            if ((((Packet *)msg) -> getSource()) == 1000)
            {
                ev << "RelaySlave£º receives REGISTER_REPLY packet from master node, process it\n";
                myMasterAddress = (((Packet *)msg) -> getSource());
                ev << "RelaySlave: my master's address is updated to "<< myMasterAddress << " according to the the REGPREPLY packet. \n";
                break;
            }
            else if (((((Packet *)msg) -> getSource()) >= 2000) & ((((Packet *)msg) -> getSource()) < 3000))
            {
                ev << "RelaySlave£º receives REGISTER_REPLY packet from relay node, process it\n";
                myRelayAddress = (((Packet *)msg) -> getSource());
                ev << "SlaveCore: my relay's address is updated to "<< myRelayAddress << " according to the the REGPREPLY packet. \n";
                break;
            }
            else if ((((Packet *)msg) -> getSource()) >= 3000)
            {
                ev << "RelaySlave: receives REGISTER_REPLY packet from slave node, RelaySlave does nothing. \n";
                break;
            }
            else
            {
                ev << "RelaySlave: the received REGREPLY packet is invalid. \n";
                break;
            }
        }

        case SYNC:
        {
            if ((((Packet *)msg) -> getSource()) == 1000)
            {
                ev << "RelaySlave: receives SYNC packet from master node, process it\n";

                // get the measurement offset based on the reception of SYNC from master,
                // and no need to use the 'AddressOffset.#
                ev << "RelaySlave: get the offset and skew...\n";

                pClock -> setReceivedSYNCTime((((Packet *)msg) -> getTsRx()));

                ev << "RelaySlave: timestamp is "<< (((Packet *)msg) -> getTsRx()) << endl;

                EstimatedOffset = pClock -> getMeasurementOffset(1, 0);
                EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);

                ev << "RelaySlave: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "RelaySlave: Done.\n";

                break;

            }
            else if (((((Packet *)msg) -> getSource()) >= 2000) & ((((Packet *)msg) -> getSource()) < 3000))
            {
                ev << "RelaySlave: receives SYNC packet from relay node, process it\n";

                ev << "RelaySlave: get the offset and skew...\n";
                AddressOffset = myMasterAddress - myAddress;

                pClock -> setReceivedSYNCTime((((Packet *)msg) -> getTsRx()));

                ev << "RelaySlave: timestamp is "<< (((Packet *)msg) -> getTsRx()) << endl;

                if (AddressOffset > 0)
                {
                    EstimatedOffset = pClock -> getMeasurementOffset(2, AddressOffset);
                    EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);
                }
                if (AddressOffset < 0)
                {
                    AddressOffset = - AddressOffset;
                    EstimatedOffset = pClock -> getMeasurementOffset(3, AddressOffset);
                    EstimatedSkew = pClock -> getMeasurementSkew(EstimatedOffset);
                }

                ev << "RelaySlave: adjust clock...\n";
                pClock -> adjustClock(EstimatedOffset, EstimatedSkew);

                ev << "RelaySlave: Done.\n";

                break;

            }
            else if (myMasterAddress >= 3000)
            {
                ev << "RelaySlave: receives SYNC packet from slave node, ignore it\n";
                break;
            }
            else
            {
                ev << "RelaySlave: the received SYNC packet is invalid, ignore it \n";
                break;
            }
        }

        case DRES:
        {
            ev << "RelaySlave: Invalid message. Ignored\n";

            // activate the second-hop time sync immediately.
            // pRelayMaster -> startSync();
        }

        default:
        {
            error("RelayMaster: receives unknown message, report warning and ignore. \n");
            break;
        }

    }
}

void RelaySlave::recordResult()
{

}

void RelaySlave::finish()
{

}

void RelaySlave::updateDisplay()
{
	char buf[100];
	// sprintf(buf, "dms [ms]: %3.2f \ndsm [ms]: %3.2f \ndpr [ms]: %3.2f \noffset [ms]: %3.2f\n ",
	// 	dms*1000,dsm*1000,dprop*1000,offset*1000);
	getDisplayString().setTagArg("t",0,buf);
}

cModule *RelaySlave::findHost(void)
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



