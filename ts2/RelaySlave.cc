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
// #include "Constant.h"


Define_Module(RelaySlave);

void RelaySlave::initialize()
{
    ev<< "Relay Slave Initialisation "<<endl;

	dpropVec.setName("prop");
	dmsVec.setName("dms");
	dsmVec.setName("dsm");
	offsetVec.setName("nodo_offset");
	driftVec.setName("nodo_drift");
	delayVec.setName("delay");
	delta_t41Vec.setName("delta_t41");
	TrVec.setName("Tr");

	/* Inizializzazione variabili di stato. */
	Tcamp  = par("Tcamp");
	//alpha = par("alpha");
	//beta = par("beta");
	//Tsync = par("Tsync");
	ts2 = ts1 =	ts3 =	ts4 = 0;
	dprop  = dms = dsm  = 0;
	drift = 0;
	if(ev.isGUI()){updateDisplay();}

	nbSentDelayRequests = 0;
	nbReceivedSyncsFromMaster = 0;
	nbReceivedDelayResponsesFromMaster = 0;
	nbReceivedSyncsFromRelay = 0;
	nbReceivedDelayResponsesFromRelay = 0;

	// for PCO
	ThresholdAdjustValue = 0;
	ClockTime = 0;
	RegisterThreshold = 0;
    // slotDuration = par("slotDuration");
    // ScheduleOffset = par("ScheduleOffset");

	/* Lettura dei parametri di ingresso. */
	//Tsync = par("Tsync");
	//Tcamp  = par("Tcamp");
	/*Parametri servo clock*/
	Ts = Ts_correct = Tm = Tm_previous = Ts_previous = 0;
	offset_previous=0;

	/* Initialise the pointer to the clock module */
	pClock = (Clock2 *)getParentModule()->getParentModule()->getSubmodule("clock");
	if (pClock==NULL)
	    error("No clock module is found in the module");

	/* Initialise the addresses of slave & master */
	name = "slave";

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
    cModule *SinkModule = findHost()->getParentModule();
    ev<<"Relay Slave: SinkModule: findHost()->getParentModule returns: "<< SinkModule->getName() <<endl;
    // Relay Slave: findHost()->getParentModule returns: TSieee802154SM

    // find Sink node (or master node)
    SinkModule = SinkModule->getSubmodule("sink");

    cModule *MasterModule = findHost()->getParentModule();
    ev<<"Relay Slave: MasterModule: findHost()->getParentModule returns: "<< MasterModule->getName() <<endl;
    // Relay Slave: findHost()->getParentModule returns: TSieee802154SM

    // find Relay node
    MasterModule = MasterModule->getSubmodule("relay", (findHost()->getIndex()));

    if (SinkModule != NULL)
    {
        myMasterAddress = 1000;
        ev<<"Relay Slave: Master default address is "<< myMasterAddress <<endl;
        ev<<"Relay Slave: my master node is "<< SinkModule->getName() <<endl;
    }
    else if (MasterModule != NULL)
    {
        // master = 2000 + ((MasterModule->getIndex()) * 1000);
        myMasterAddress = 2000 + (MasterModule->getIndex());
        ev<<"Relay Slave: Master default address is "<< myMasterAddress <<endl;
        ev<<"Relay Slave: my master node is "<< MasterModule->getName() <<endl;
    }
    else
    {
        error("No Sink node or RelayMaster are found");
    }

	// Find the Relay Master, and Relay Master pointer
	// ToDo: need to change the getSubmodule from ptpM2 to RelayMaster (Done by Yan Zong)
	pRelayMaster = (RelayMaster *)getParentModule()->getSubmodule("RelayMaster");
	ev<<"Relay Master: Relay Master pointer is "<< pRelayMaster <<endl;
    if (pRelayMaster==NULL)
    {
        error("could not find Relay Master module in a Relay node (boundary clock node)");
    }

    // NodeId = (findHost()->getIndex() + 1);
    // EV << "RelaySlave: the node id is " << NodeId << endl ;
    // id of relay[0] should be 1; id of relay[1] should be 2;

	// Register with Master node
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
	    // scheduleAt(simTime() + NodeId*slotDuration + ScheduleOffset, new cMessage("OffsetTimer"));

	    // handleClockMessage(msg);
	}

	if (msg->arrivedOn("in"))   // data packet from lower layer
	{   // check PtpPkt type
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

    EV << "Relay Slave generates a NEW SYNC Packet \n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    pck->setByteLength(40); // SYNC_BYTE = 40

    pck->setTimestamp(simTime());   // time stamp

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
		pck->setSource(myAddress);
		pck->setDestination(((Event *)msg)->getDest());
		pck->setByteLength(((Event *)msg)->getPckLength());
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

    switch(((PtpPkt *)msg)->getPtpType()){
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
            // for PCO (Pulse-Coupled Oscillator)
            ev << "Relay Slave receives SYNC packet from master node, process it\n";
            ev << "Relay Slave: adjust the threshold of clock...\n";

            // pRelayMaster->startSync();  // broadcast SYNC packet

            // servo_clock();  // adjust the clock
            break;

            // for PTP, there is no need use these in the PCO (Pulse-Coupled Oscillator)
            /*
            ev << "Relay Slave receives SYNC packet from master node, process it\n";
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

            nbReceivedSyncsFromMaster = nbReceivedSyncsFromMaster + 1;  // count the number of the received SYNC packet.
            break;
            */
        }
        else if (myMasterAddress == 2000 || myMasterAddress > 2000)
        {
            // for PCO (Pulse-Coupled Oscillator)
            ev << "Relay Slave receives SYNC packet from relay node, ignore it\n";
            // ev << "Relay Slave receives SYNC packet from relay node, process it\n";

            // servo_clock();  // adjust the clock
            break;

            // for PTP, there is no need use these in the PCO (Pulse-Coupled Oscillator)
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

            nbReceivedSyncsFromRelay = nbReceivedSyncsFromRelay + 1;  // count the number of the received SYNC packet.
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

            nbReceivedDelayResponsesFromMaster = nbReceivedDelayResponsesFromMaster + 1;

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

            nbReceivedDelayResponsesFromRelay = nbReceivedDelayResponsesFromRelay + 1;

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

// called in the handleSelfMessage() function
void RelaySlave::ProduceT3packet()
{
    ev << "Relay Slave: debug: (in DREQ) myMasterAddress = " << myMasterAddress <<".\n";
    ev << "Relay Slave handleMasterMesage() is processing DRES Packet from Master.\n";
    ts3 = pClock->getTimestamp();
    ev << "ts3 = "<< ts3 <<endl;
    PtpPkt *pck = new PtpPkt("DREQ");
    pck->setByteLength(40);    // DREQ_BYTE = 40
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

    nbSentDelayRequests = nbSentDelayRequests + 1;
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
    EV << "nbReceivedSyncsFromMaster = " << nbReceivedSyncsFromMaster << endl;
    EV << "nbReceivedSyncsFromRelay = " << nbReceivedSyncsFromRelay << endl;
    EV << "nbReceivedDelayResponsesFromMaster = " << nbReceivedDelayResponsesFromMaster << endl;
    EV << "nbReceivedDelayResponsesFromRelay = " << nbReceivedDelayResponsesFromRelay << endl;
    EV << "nbSentDelayRequests = " << nbSentDelayRequests << endl;

    recordScalar("nbReceivedSyncsFromMaster", nbReceivedSyncsFromMaster);
    recordScalar("nbReceivedSyncsFromRelay", nbReceivedSyncsFromRelay);
    recordScalar("nbReceivedDelayResponsesFromMaster", nbReceivedDelayResponsesFromMaster);
    recordScalar("nbReceivedDelayResponsesFromRelay", nbReceivedDelayResponsesFromRelay);
    recordScalar("nbSentDelayRequests", nbSentDelayRequests);
}

void RelaySlave::updateDisplay()
{
	char buf[100];
	sprintf(buf, "dms [ms]: %3.2f \ndsm [ms]: %3.2f \ndpr [ms]: %3.2f \noffset [ms]: %3.2f\n ",
		dms*1000,dsm*1000,dprop*1000,offset*1000);
	getDisplayString().setTagArg("t",0,buf);
}


// ---------------------------------------------------------------------------
// Servo Clock used to update the local drifting clock
// ---------------------------------------------------------------------------


void RelaySlave::servo_clock()
{
    pClock -> adjustThreshold();
}




//void RelaySlave::servo_clock()
//{
//	dms = ts2 - ts1;
//	dsm = ts4 - ts3;
//	ev<<"dms="<<dms<<"dsm="<<dsm<<endl;
//	dprop = (dms + dsm)/2;
//	offset = dms - dprop;
//	//offset = ((ts2 - ts1)- (ts4 - ts3))/2;
//	ev<<"offset="<<offset<<endl;
//   //double alpha = 1;
//    //double beta = 200;
//    //double y =offset/alpha;
//	//ev<<"y="<<y<<endl;
//    ev<<"offset= "<<offset<<endl;
//
/*    // the following codes of packet exchange will be
    // replace by calling function clock2::adjtimeex() directly
    Packet *pck = new Packet("ADJ_OFFSET");
	pck->setPckType(CLOCK);
	pck->setClockType(OFFSET_ADJ);
    //pck->setData(y);
	pck->setData(offset);
	send(pck,"outclock");
*/
//
//
//	Ts = ts2;//T2
//	Tm = ts1;//T1
//	ev<<"Ts-Ts_previous="<<Ts-Ts_previous<<endl;
//	//if(Tm_previous > 0){  //去掉这个if判断语句，不然drift在第一次同步周期内没有得到校正,去掉之后对结果没什么影响
//	// TODO:drift estimate
//		//drift = (Ts-Ts_previous+y)/(Tm-Tm_previous)-1;//从物理意义上来说，这个计算drift的算法是错误的
//	//drift = (Ts - Ts_previous + offset_previous)/(Tm-Tm_previous)-1;//仿真结果最好的一个算法
//	//drift = (Ts - Ts_previous)/(Tm-Tm_previous)-1;//在我的时钟模型中，这个算法是不对的,应在clock中加上u[0][0]/Tsync
//   drift = (offset-offset_previous)/(Tm-Tm_previous);//仿真结果最差，这个算法是不对的,应在在clock中加上u[0][0]/Tsync;为了对应KF参数，采用此算法
//	//drift = offset/(Tm-Tm_previous);
	/*double DELTADRIFT=10E-6;
		if(drift>DELTADRIFT){drift=DELTADRIFT;}
		else if(drift<-DELTADRIFT){drift=-DELTADRIFT;}//这个判定值与物理时钟drift的初始值有关*/
//	//double x= drift/beta;
//		ev<<"drift="<<drift<<endl;
//
/*    // the following codes of packet exchange will be
	  // replace by calling function clock2::adjtimeex() directly
		Packet *pckd = new Packet("ADJ_FREQ");
		pckd->setPckType(CLOCK);
		pckd->setClockType(FREQ_ADJ);
		//pckd->setData(x);
		pckd->setData(drift);
		ev<<"drift="<<drift<<endl;
		send(pckd,"outclock");
	//}
*/
//	//dxw->hyw: see my question in Clock2.cc, line 225
//	pClock->adjtimex(offset, 0);    // calculate the offset adjust value
//	pClock->adjtimex(drift,1);  // calculate the drift adjust value
//	pClock->adj_offset_drift(); // adjust the clock offset and drift
//
//	recordResult();
//
//    offset_previous = offset;
//	Tm_previous = Tm;
//	Ts_previous = Ts;
//
//
//}



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



