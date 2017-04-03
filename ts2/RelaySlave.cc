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

#include "RelaySlave.h"
#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "ApplPkt_m.h"

Define_Module(RelaySlave);

void RelaySlave::initialize()
{
    ev << "RelaySlave Initialisation \n";
	dpropVec.setName("prop");
	dmsVec.setName("dms");
	dsmVec.setName("dsm");
	offsetVec.setName("nodo_offset");
	driftVec.setName("nodo_drift");
	delayVec.setName("delay");
	delta_t41Vec.setName("delta_t41");
	TrVec.setName("Tr");
	Tcamp  = par("Tcamp");

	ts2 = ts1 =	ts3 = ts4 = 0;
	dprop  = dms = dsm  = 0;
	drift = 0;
	Ts = Ts_correct = Tm = Tm_previous = Ts_previous = 0;
	offset_previous = 0;

	if(ev.isGUI())
	{
	    updateDisplay();
	}

	/* Initialise the pointer to the clock module */
	pClock = (PCOClock *)getParentModule()->getParentModule()->getSubmodule("clock");
	if (pClock==NULL)
	    error("No clock module is found in the module");

	// ---------------------------------------------------------------------------
    // ArpHost module parameters
    // parameter 'slaveAddrOffset' is used to set the address of the slave (i.e., S1, S2, etc)
    // set the slave address, using the same IP, MAC address as ArpHost (see the *.ini file)
    // ---------------------------------------------------------------------------
	if (hasPar("slaveAddrOffset"))
	    myAddress = (findHost()->getIndex()) + (int)par("slaveAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
	else
	    error("No parameter slaveAddrOffset is found");

    ev << "Relay Slave address is "<< myAddress <<endl;

	// find Sink node (or Master node), and Relay node, and their respective address
    cModule *MasterModule = findHost()->getParentModule();
    ev<<"Relay Slave: MasterModule: findHost()->getParentModule returns: "<< MasterModule->getName() <<endl;
    // Relay Slave: findHost()->getParentModule returns: TSieee802154SM

    // find master node
    MasterModule = MasterModule->getSubmodule("mnode");

    cModule *RelayModule = findHost()->getParentModule();
    ev<<"Relay Slave: RelayModule: findHost()->getParentModule returns: "<< RelayModule->getName() <<endl;
    // Relay Slave: findHost()->getParentModule returns: TSieee802154SM

    // find Relay node
    RelayModule = RelayModule->getSubmodule("rnode", (findHost()->getIndex()));

    if (MasterModule != NULL)
    {
        myMasterAddress = 1000;
        ev<<"Relay Slave: Master default address is "<< myMasterAddress <<endl;
        ev<<"Relay Slave: my master node is "<< MasterModule->getName() <<endl;
    }
    else if (RelayModule != NULL)
    {
        myMasterAddress = 2000 + (RelayModule->getIndex());
        ev<<"Relay Slave: Master default address is "<< myMasterAddress <<endl;
        ev<<"Relay Slave: my master node is "<< RelayModule->getName() <<endl;
    }
    else
    {
        error("No Sink node or RelayMaster are found");
    }

	// Find the Relay Master, and Relay Master pointer
	// ToDo: need to change the getSubmodule from ptpM2 to RelayMaster (Done by Yan Zong)
	pRelayMaster = (RelayMaster *)getParentModule()->getSubmodule("RelayMaster");
	ev<<"Relay Master: Relay Master pointer is "<< pRelayMaster <<endl;
    if (pRelayMaster == NULL)
    {
        error("could not find Relay Master module in a Relay node (boundary clock node)");
    }

	PtpPkt * temp = new PtpPkt("REGISTER");
	temp->setPtpType(REG);
	temp->setByteLength(0);

    // use the host modules findHost() as a appl address
    temp->setSource(myAddress);
    temp->setDestination(PTP_BROADCAST_ADDR); //PTP_BROADCAST_ADDR = -1

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    temp->setDestAddr(LAddress::L3BROADCAST);
    temp->setSrcAddr(myAddress);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );

    send(temp,"out");
	ev << "Relay Slave: initialisation finished, send REGISTER \n";
}

// msg is always deleted at the end of handlMessage(), but the temp(msg) is not deleted at the end of initialize()
void RelaySlave::handleMessage(cMessage *msg)
{
	ev << "Relay Slave: handleMessage invoked\n";

    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
        delete msg;
    }

	if (msg->arrivedOn("inclock"))
	{
	    EV << "Relay Slave receives a SYNC packet from clock module, delete it and re-generate a full SYNC packet \n";
	    delete msg;

	    scheduleAt(simTime(), new cMessage("OffsetTimer"));
	    // handleClockMessage(msg);
	}

	if (msg->arrivedOn("in"))   // data packet from lower layer
	{
        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {
            EV << "Relay Slave receives a PtpPkt packet. ";
            if(((PtpPkt*)msg)->getSource()!=myAddress  &
                     (((PtpPkt*)msg)->getDestination()==myAddress  |
                             ((PtpPkt*)msg)->getDestination()==PTP_BROADCAST_ADDR))  // PTP_BROADCAST_ADDR = -1
            {
                EV << "the PtpPkt packet is for me, Relay Slave is processing it\n";
                handleMasterMessage(msg);
            }
            else
                EV << "the PtpPkt packet is not for me, ignore it, do nothing\n";
            delete msg;
        }
        else
        {
            EV << "the PtpPkt packet is not for me, send it up to higher layer\n"<<endl;
            // send(((ApplPkt *)msg)->dup(),"upperGateOut");
            send(msg, "upperGateOut");
         }
	}

    if (msg->arrivedOn("upperGateIn"))   // data packet from upper layer
    {
         EV << "receive a PtpPkt packet from higher layer"<<endl;
         // send(((ApplPkt *)msg)->dup(),"out");
         send(msg, "out");
    }

	if (msg->arrivedOn("inevent"))
	{
	    handleEventMessage(msg);
	    delete msg;
	}

	// delete msg;

	if(ev.isGUI())
	{
	    updateDisplay();
	}
}

void RelaySlave::handleSelfMessage(cMessage *msg)
{
    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    pck->setByteLength(44);

    pck->setTimestamp(simTime());

    pck->setSource(myAddress);
    pck->setDestination(PTP_BROADCAST_ADDR);

    pck->setData(SIMTIME_DBL(simTime()));
    pck->setTsTx(SIMTIME_DBL(simTime())); // set transmission time stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setSrcAddr( LAddress::L3Type(myAddress));
    pck->setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );

    EV << "Relay Slave broadcasts SYNC packet" << endl;
    send(pck,"out");

    // ProduceT3packet();
}

void RelaySlave::handleOtherPacket(cMessage *msg)
{
}

void RelaySlave::handleEventMessage(cMessage *msg)
{
	if(((Event *)msg)->getEventType()==CICLICO)
	{
		Packet *pck = new Packet("CICLICO");
		pck->setPckType(OTHER);
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

void RelaySlave::handleClockMessage(cMessage *msg)
{
    error("Now node does not exchange packet with clock. Used to acquire time stamps by packet exchange. Now we use function calls.");
}

void RelaySlave::handleMasterMessage(cMessage *msg)
{
    myMasterAddress = (((PtpPkt *)msg)->getSource());

    switch(((PtpPkt *)msg)->getPtpType())
    {
        case REG:
        {
            ev << "receive REGISTER package, slave node does nothing. returns\n";
            break;
        }

        case REGRELAYMASTER:
        {
            ev << "receive REGISTER package, slave node does nothing. returns\n";
            break;
        }

        case REGREPLY:
        {
            if (myMasterAddress == 1000)
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from master node, process it\n";
                ev << "my master address is updated to "<< myMasterAddress <<endl;
                break;
            }
            else if (myMasterAddress == 2000 || myMasterAddress > 2000)
            {
                ev << "Relay Slave receives REGISTER_REPLY packet from Relay node, process it\n";
                ev << "my master address is updated to "<< myMasterAddress <<endl;
                break;
            }
            else
            {
                ev << "Relay Slave: the received REGREPLY packet is invalid, ignore it \n";
                break;
            }
        }

        case SYNC:
        {
            if (myMasterAddress == 1000)
            {
                // ---------------------------------------------------------------------------
                // for PCO
                // ---------------------------------------------------------------------------
                ev << "Relay Slave receives SYNC packet from master node, process it\n";
                ev << "Relay Slave: adjust the threshold of clock...\n";

                // servo_clock();  // adjust the clock

                pClock -> getThresholdOffsetWithMaster();

                pClock -> adjustThresholdBasedMaster();

                break;

                // ---------------------------------------------------------------------------
                // for PCO
                // ---------------------------------------------------------------------------
                /*
                ev << "Relay Slave receives SYNC packet from master node, process it\n";
                ts1 = ((PtpPkt *)msg)->getTsTx();

                ev << " Relay Slave: ts1 = "<< ts1 <<endl;
                ev << " Relay Slave: the arrival time of SYNC = "<< SIMTIME_DBL(simTime()) <<endl;
                pClock->setT123(ts1,0,0);

                Tr = ((Packet *)msg)->getByteLength()*8; //包的传输时延,dxw->hyw: what is Tr for?

                // update the address of master according the received SYNC
                ev << "my master address is updated to "<< myMasterAddress <<endl;

                // New codes that call public function clock::getTimeStamp()
                ts2 = pClock->getTimestamp();  //DXW20150129: TODO: shall we use the packet's getTsRx()
                                                // to get the packet's Rx timestamp by the ptpStmp module
                                                //  or use the timestamp at higher layer (e.g. ptpSlave.Node)
                ev << " Relay Slave: ts2 = "<< ts2 <<endl;
                T= 1/rate;
                // delay = uniform(0,Tcamp/10);//t2包和t3包间的处理时延
                delay = uniform(0,1E-5);
                // delay=1E-5;
                // ToDo: double check whether need scheduleAt (Done by Yan Zong).
                // this scheduleAt use to generate the DREQ packet.
                scheduleAt(simTime()+delay, new cMessage("SLtimer"));
                delayVec.record(delay); //记录t2,t3间的时延

                ev << "delay (t2 to t3)= " << delay <<endl;

                nbReceivedSyncsFromMaster = nbReceivedSyncsFromMaster + 1;  // count the number of the received SYNC packet.
                break;
                */
            }
            else if (myMasterAddress == 2000 || myMasterAddress > 2000)
            {
                // ---------------------------------------------------------------------------
                // for PCO
                // ---------------------------------------------------------------------------
                ev << "Relay Slave receives SYNC packet from relay node, ignore it\n";
                ev << "Relay Slave: adjust the threshold of clock...\n";

                // servo_clock();  // adjust the clock

                pClock -> getThresholdOffsetWithRelay();

                pClock -> adjustThresholdBasedRelay();

                break;

                // ---------------------------------------------------------------------------
                // for PTP
                // ---------------------------------------------------------------------------
                /*
                ev << "Relay Slave receives SYNC packet from relay node, process it\n";
                ts1 = ((PtpPkt *)msg)->getTsTx();

                ev << " Relay Slave: ts1 = "<< ts1 <<endl;
                ev << " Relay Slave: the arrival time of SYNC = "<< SIMTIME_DBL(simTime()) <<endl;
                pClock->setT123(ts1,0,0);

                Tr= ((Packet *)msg)->getByteLength()*8; //包的传输时延,dxw->hyw: what is Tr for?

                // update the address of master according the received SYNC
                ev << "my master address is updated to "<< myMasterAddress <<endl;

                // New codes that call public function clock::getTimeStamp()
                ts2 = pClock->getTimestamp();  //DXW20150129: TODO: shall we use the packet's getTsRx()
                                                        // to get the packet's Rx timestamp by the ptpStmp module
                                                        //  or use the timestamp at higher layer (e.g. ptpSlave.Node)
                ev << " Relay Slave: ts2 = "<< ts2 <<endl;
                T= 1/rate;
                // delay = uniform(0,Tcamp/10);//t2包和t3包间的处理时延
                delay = uniform(0,1E-5);
                // delay=1E-5;
                // ToDo: double check whether need scheduleAt (Done by Yan Zong).
                // this scheduleAt use to generate the DREQ packet.
                scheduleAt(simTime()+delay, new cMessage("SLtimer"));
                delayVec.record(delay); //记录t2,t3间的时延

                ev << "delay (t2 to t3)= " << delay <<endl;

                break;
                */
            }
            else
            {
                ev << "Relay Slave: the received SYNC packet is invalid, ignore it \n";
                break;
            }
        }

        case DRES:
        {
            if (myMasterAddress == 1000)
            {
                ev << "Relay Slave handleMasterMesage() is processing DRES Packet from Master.\n";
                ts4 = ((PtpPkt *)msg)->getData();
                ev << " Relay Slave: ts4 = "<< ts4 <<endl;
                delta_t41 = SIMTIME_DBL(simTime()) - ts1;
                servo_clock();

                // activate the second-hop time sync immediately.
                // pRelayMaster->startSync();
                break;
            }
            else if (myMasterAddress == 2000 || myMasterAddress > 2000)
            {
                ev << "Relay Slave handleMasterMesage() is processing DRES Packet from relay node.\n";
                ts4 = ((PtpPkt *)msg)->getData();
                ev << " Relay Slave: ts4 = "<< ts4 <<endl;
                delta_t41 = SIMTIME_DBL(simTime()) - ts1;
                servo_clock();

                // activate the second-hop time sync immediately.
                // pRelayMaster->startSync();
                break;
            }
            else
            {
                ev << "Relay Slave: the received DRES packet is invalid, ignore it \n";
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

void RelaySlave::ProduceT3packet()
{
    ts3 = pClock->getTimestamp();
    ev << "ts3 = "<< ts3 <<endl;
    PtpPkt *pck = new PtpPkt("DREQ");
    pck->setByteLength(40);
    pck->setPtpType(DREQ);

    pck->setDestination(myMasterAddress);
    pck->setSource(myAddress);

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setDestAddr(LAddress::L3Type(myMasterAddress));
    pck->setSrcAddr(LAddress::L3Type(myAddress));

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3Type(myMasterAddress));

    send((cMessage *)pck,"out");
    ev<<"Relay Slave: transmit DREQ packet \n";
}

void RelaySlave::recordResult()
{
	dsmVec.record(dsm);
	dpropVec.record(dprop);
	dmsVec.record(dms);
	offsetVec.record(offset);
	driftVec.record(drift);
	delta_t41Vec.record(delta_t41);
	TrVec.record(Tr);
}

void RelaySlave::finish()
{
}

void RelaySlave::updateDisplay()
{
	char buf[100];
	sprintf(buf, "dms [ms]: %3.2f \ndsm [ms]: %3.2f \ndpr [ms]: %3.2f \noffset [ms]: %3.2f\n ",
		dms*1000,dsm*1000,dprop*1000,offset*1000);
	getDisplayString().setTagArg("t",0,buf);
}

// ---------------------------------------------------------------------------
// this function is used to adjust the clock
// ---------------------------------------------------------------------------
void RelaySlave::servo_clock()
{

    pClock -> getThresholdOffsetWithMaster();

    pClock -> adjustThresholdBasedMaster();

    // pClock -> adjustThresholdBasedRelay();
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



