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

#include <string.h>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Event_m.h"
#include "Constant.h"
#include "PtpPkt_m.h"
#include "ApplPkt_m.h"
#include "RelayBuffer.h"


Define_Module(RelayBuffer);

void RelayBuffer::initialize()
{
	// ---------------------------------------------------------------------------
	// Ts rappresenta il tempo che impiega il buffer per trasmettere il pacchetto,
	//    questo tempo  viene calcolato come rapporto tra la lunghessa in bit del
	//    pacchetto e bit rate dello switch.
	/*Ts表示传输的数据包在缓冲区所花费的时间，这时间的计算方法在开关的数据包的位和位速率之间的比率lunghessa。*/
	// ---------------------------------------------------------------------------
	ev << "RelayBuffer Initialisation \n";
	queue.setName("buffer");
	pckVec.setName("pacchetti");//pacchetti,包
	byteVec.setName("byte");
	pacchetti = 0;			 // numero di pacchetti in coda  队列/堆栈里的数据包
	byte = 0;				 // numero di byte occupati 占用的字节数
	Ts = 0;				 // tempo impiegato per spedire un pacchetto  发送一个数据包花费的时间
	rate=par("rate");	 // [bit/s]
	if(rate==0)
		T=0;
	else
		T=1/rate;
	Tlat=par("latenza"); // [s] latenza: impostare a zero 中断延时,设置为零
	if(ev.isGUI()){updateDisplay();}
	ev << "RelayBuffer Initialisation Success\n";
}

void RelayBuffer::handleMessage(cMessage *msg)
{
    ev << "RelayBuffer handle self message \n";
	if(msg->isSelfMessage())
	{
		ev << "RelayBuffer does not have any self message, ignore it\r\n";
		delete msg;
		return;
	}
	else{
	    ev << "RelayBuffer Kernel Module.\n";
	    ev << "RelayBuffer receives a message at " << simTime()*1000 << " [ms]\n";
	    handleRelayMessage(msg);
	    ev << "RelayBuffer forward message Relay Master or Relay Slave\n";
	}
	if(ev.isGUI()){updateDisplay();}

}

// buffer send the packet to the RelayMaster or RelaySalve, RelayMaster and RelaySalve
// determine whether ignore the received packet (except REGISTER, REPLY_REGISTER)
void RelayBuffer::handleRelayMessage(cMessage *msg)
{
    ev << "debug: gate in[2]'s idex = " <<gate("in", 2)->getIndex()<<".\n";
    ev << "debug: gate out[2]'s idex = " <<gate("out", 2)->getIndex()<<".\n";

    PtpPkt *pkt;
    if (dynamic_cast<PtpPkt *>(msg) != NULL)
    {
        ev << "the received packet is PtpPkt packet.\n";
        isPtpPkt = TRUE;
        pkt=(PtpPkt*)msg;
    }
    else
    {
        ev << "the received packet is not PtpPkt packet.\n";
        isPtpPkt = FALSE;
    }

    if (msg->arrivedOn ("in", 2 ))
    {
        if (isPtpPkt == FALSE)
        {
            // Forward non-PtpPkt packet to upper layer (network layer)
            send(msg,"out", 0); // relay slave (upper gate)
        }
        else
        {
            if (pkt->getDestination() == PTP_BROADCAST_ADDR)
            {
                send(msg,"out",0); // RelaySlave
                send((pkt)->dup(),"out",1);   // RelayMaster
            }
            else if ( (pkt->getDestination() == 2000) | (pkt->getDestination() > 2000) )
            {
                send(msg,"out",0);  // RelaySlave
                send((pkt)->dup(),"out",1);  // RelayMaster
            }
            else
            {
                ev << "RelayBuffer error in handleRelayMessagne.\n";
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
        ev << "RelayBuffer error.\n";
    }
    ev << "RelayBuffer Success.\n";
}

void RelayBuffer::finish(){
	ev << "RelayBuffer : finish...\n";
	ev << "RelayBuffer : remove all queued messages \n";
	while(!queue.empty()){
		delete (cMessage *)queue.pop();
	}
	ev << "RelayBuffer : all messages have been removed.\n";
}

void RelayBuffer::updateDisplay(){
	char buf[40];
	sprintf(buf, "pck: %d   byte: %d\n",pacchetti,byte);
	getDisplayString().setTagArg("t",0,buf);
}
