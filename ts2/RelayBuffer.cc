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

#include <string.h>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Event_m.h"
#include "Constant.h"
#include "ApplPkt_m.h"
#include "RelayBuffer.h"

Define_Module(RelayBuffer);

void RelayBuffer::initialize()
{
	ev << "RelayBuffer Initialisation... \n";
	pckVec.setName("package");  // package

	if(ev.isGUI())
	{
	    updateDisplay();
	}

	ev << "RelayBuffer Initialisation Success\n";
}

void RelayBuffer::handleMessage(cMessage *msg)
{
	if(msg -> isSelfMessage())
	{
		ev << "RelayBuffer: RelayBuffer does not have any self message, ignore and delete it\r\n";
		delete msg;
		return;
	}
	else
	{
	    ev << "RelayBuffer: RelayBuffer receives a message. \n";
	    handleRelayMessage(msg);
	    ev << "RelayBuffer: RelayBuffer forwards message RelayMaster or RelaySlave\n";
	}

	if(ev.isGUI())
	{
	    updateDisplay();
	}

}

// buffer send the packet to the RelayMaster or RelaySalve, RelayMaster and RelaySalve
// determine whether ignore the received packet
void RelayBuffer::handleRelayMessage(cMessage *msg)
{
    Packet *pkt;
    if (dynamic_cast<Packet *>(msg) != NULL)
    {
        ev << "the received packet is PtpPkt packet.\n";
        isPtpPkt = TRUE;
        pkt = (Packet*)msg;
    }
    else
    {
        ev << "the received packet is not PtpPkt packet.\n";
        isPtpPkt = FALSE;
    }

    if (msg->arrivedOn ("in", 2))
    {
        if (isPtpPkt == FALSE)
        {
            // Forward non-PtpPkt packet to upper layer (network layer)
            send(msg,"out", 0); // relay slave (upper gate)
        }
        else
        {
            if (pkt -> getDestination() == PACKET_BROADCAST_ADDR)
            {
                send(msg,"out",0); // RelaySlave
                send((pkt)->dup(),"out",1);   // RelayMaster
            }
            else if ( (pkt -> getDestination() >= 2000) & (pkt -> getDestination() < 3000) )
            {
                send(msg,"out",0);  // RelaySlave
                send((pkt)->dup(),"out",1);  // RelayMaster
            }
            else
            {
                error("RelayBuffer: error in handleRelayMessagne");
            }
        }
    }

    else if (msg->arrivedOn ("in", 1 ))
    {
        if (isPtpPkt == FALSE)
        {
            // Forward non-PtpPkt packet to lower layer
            send(msg,"out", 2); // lower gate
        }
        else
        {
            send(msg,"out",2);
        }
    }

    else if ((msg->arrivedOn ("in", 0 )))
    {
        if (isPtpPkt == FALSE)
        {
            // Forward non-PtpPkt packet to lower layer
            send(msg,"out", 2); // lower gate
        }
        else
        {
            send(msg,"out",2);
        }
    }

    else
    {
        error("RelayBuffer: error in handleRelayMessagne");
    }
    ev << "RelayBuffer: success.\n";
}

void RelayBuffer::finish()
{
	ev << "RelayBuffer: finish...\n";
}

void RelayBuffer::updateDisplay()
{
	char buf[40];
	sprintf(buf, "pck: %d \n",package);
	getDisplayString().setTagArg("t",0,buf);
}
