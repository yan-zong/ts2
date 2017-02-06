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


#include "Slave.h"
#include <assert.h>
#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "Constant.h"
Define_Module(Slave);

void Slave::initialize()
{

	dpropVec.setName("prop");
	dmsVec.setName("dms");
	dsmVec.setName("dsm");
	offsetVec.setName("nodo_offset");
	driftVec.setName("nodo_drift");
	delayVec.setName("delay");
	delta_t41Vec.setName("delta_t41");
	TrVec.setName("Tr");

	Tcamp  = par("Tcamp");
	// alpha = par("alpha");
	// beta = par("beta");
	// Tsync = par("Tsync");
	ts2 = ts1 =	ts3 =	ts4 = 0;
	dprop  = dms = dsm  = 0;
	drift = 0;
	if(ev.isGUI()){updateDisplay();}

	// parameter for servo clock
	Ts = Ts_correct = Tm = Tm_previous = Ts_previous = 0;
	offset_previous=0;

	// Initialise the pointer to the clock module
	pClock = (Clock2 *)getParentModule()->getParentModule()->getSubmodule("clock");
	if (pClock==NULL)
	    error("No clock module is found in the module");

	 if (hasPar("slaveAddrOffset"))
	     address = findHost() -> getIndex() + (int)par("slaveAddrOffset"); // ->getId(); for compatible with MiXiM, see BaseAppLayer.cc
	 else
	     address = findHost() -> getIndex();
     ev<<"slave node: my address is "<<address<<endl;

	// find the master node and its address
	cModule *masterModule = findHost()->getParentModule();
	masterModule = masterModule->getSubmodule("mnode");
	if (masterModule==NULL)
	{
	    master = 0;
	    ev<<"WARNING: No master node named mnode is found. Using 0 as my master address\n";
	}
	master = masterModule -> getIndex();
	myMasterNode = masterModule;
	ev<<" my master's default address is "<< master <<endl;

	// Register with a master
	PtpPkt * temp = new PtpPkt("REGISTER");
	temp -> setPtpType(REG);
	temp -> setByteLength(0);

    temp -> setSource(address);
    temp -> setDestination(PTP_BROADCAST_ADDR);

    temp -> setDestAddr(LAddress::L3BROADCAST);
    temp -> setSrcAddr(address);

    // set the control info to tell the network layer the layer 3
    NetwControlInfo::setControlInfo(temp, LAddress::L3BROADCAST );
    send(temp,"out");

	ev << "Slave node : initialization finished\n";
}

// dxw: it seems the msg is always deleted at the end of handlMessage()
void Slave::handleMessage(cMessage *msg){
	ev<<"Node::handleMessage invoked\n";
    if(msg->isSelfMessage()){handleSelfMessage(msg);} //dxw->hyw: has msg been deleted inhandleSelfMessage()?
	if(msg->arrivedOn("inclock")){
	    error("dxw: new Node does not exchange Packet with clock.");
	    // handleClockMessage(msg);
	}

	if(msg->arrivedOn("in")) // data packet from lower layer
	{   // check PtpPkt type
        if (dynamic_cast<PtpPkt *>(msg) != NULL)
        {  EV<<"Received a PtpPkt packet. ";
             if((((PtpPkt*)msg)->getSource() != address) &
                     ((((PtpPkt*)msg)->getDestination() == address) |
                             (((PtpPkt*)msg)->getDestination() == PTP_BROADCAST_ADDR))) //PTP_BROADCAST_ADDR = -1
             {        EV<<"the PtpPkt is for me, process it\n";
               handleMasterMessage(msg);
             }
             else
                 EV<<"the PtpPck is not for me, ignore it. Do nothing\n";
        }
        else
        {   EV<<"NOt a PtpPkt packet, send it up to higher layer"<<endl;
            send(((PtpPkt *)msg)->dup(),"upperGateOut");
         }
	}

	if(msg->arrivedOn("inevent"))
	      {handleEventMessage(msg);}

	delete msg;  // ?? repeated delete msg?, see the 9-th line above
	if(ev.isGUI()){updateDisplay();}
}

//ToDo Modify handleSelfMessage()
void Slave::handleSelfMessage(cMessage *msg){

	           ProduceT3packet();

}
void Slave::handleOtherPacket(cMessage *msg){}

void Slave::handleEventMessage(cMessage *msg){
	if(((Event *)msg)->getEventType()==CICLICO){
		Packet *pck = new Packet("CICLICO");
		pck->setPckType(OTHER);
		pck->setSource(address);
		pck->setDestination(((Event *)msg)->getDest());
		pck->setByteLength(((Event *)msg)->getPckLength());
		for(int i=0; i<((Event *)msg)->getPckNumber()-1; i++){
				send((cMessage *)pck->dup(),"out");
			}
		send(pck,"out");
	}
}


void Slave::handleClockMessage(cMessage *msg){
    error("Now node does not exchange packet with clock. Used to acquire time stamps by packet exchange. Now we use function calls.");
   }

void Slave::handleMasterMessage(cMessage *msg){
    switch(((PtpPkt *)msg)->getPtpType()){
		case SYNC:
		{	//ts1 = ((PtpPkt *)msg)->getData();
		    ts1 = ((PtpPkt *)msg)->getTsTx();


            ev<<"   ts1="<<ts1<<endl;
            ev<<"The arival time of SYNC="<<SIMTIME_DBL(simTime())<<endl;
            pClock->setT123(ts1,0,0);

            Tr= ((Packet *)msg)->getByteLength()*8; //包的传输时延,dxw->hyw: what is Tr for?

			// update the address of my master according the received SYNC
            master=((PtpPkt *)msg)->getSource();

        // New codes that call public function clock::getTimeStamp()
            ts2 = pClock->getTimestamp();  //DXW20150129: TODO: shall we use the packet's getTsRx()
                                           // to get the packet's Rx timestamp by the ptpStmp module
                                           //  or use the timestamp at higher layer (e.g. ptpSlave.Node)
            // the following are the same codes as handleClockMessage(), case SYNC
            ev<<"The time of T2:"<<endl;
            ev<<"T2="<<ts2<<endl;
            T= 1/rate;
            // delay = uniform(0,Tcamp/10);//t2包和t3包间的处理时延
            delay = uniform(0,1E-5);
            //delay=1E-5;
            scheduleAt(simTime()+delay, new cMessage("SLtimer"));
            delayVec.record(delay);//记录t2,t3间的时延
            ev<<"delay (t2 to t3)= "<<delay<<endl;
            break;

       /*     // Old codes that exchanges Packet with clock to get time stampe.
            // dxw: it seems that the clock does not response PtpPkt type packet.
            // TODO: New Packet("SYN_TIME_REQ") and copy all the properties
            //       from PtpPkt to Packet
            Packet *clkpck=new Packet("SYN_TIME_REQ"); // replace msg->setName("SYN_TIME_REQ");
            clkpck->setClockType(TIME_REQ);
            clkpck->setPckType(((PtpPkt *)msg)->getPckType());
			clkpck->setPtpType(((PtpPkt *)msg)->getPtpType());
			clkpck->setSource(((PtpPkt *)msg)->getSource());
			clkpck->setDestination(((PtpPkt *)msg)->getDestination());
			clkpck->setData(((PtpPkt *)msg)->getData());
			clkpck->setByteLength(((PtpPkt *)msg)->getByteLength());

			send((cMessage *)clkpck,"outclock");// replace send((cMessage *)msg->dup(),"outclock");
		*/
			break;
		}
		case DRES:
			ev<<"Node handleMasterMesage processes DRES Packet from Master.\n";
			ts4 = ((PtpPkt *)msg)->getData();
			ev<<"The time of T4:"<<endl;
            ev<<"T4="<<ts4<<endl;
            delta_t41 = SIMTIME_DBL(simTime()) - ts1;
			servo_clock();
			break;
		case REG:
			ev<<"Register package, slave node does nothing. returns\n";
			break;
        case REGREPLY:
        {   ev<<"Received register_reply packet (PtpType=REGREPLY). \n";
            ev<<"Update my master's address from "<< master;
            master=(((PtpPkt *)msg)->getSource());
            ev<<" to"<< master << " according to the the REGPREPLY packet)\n";
            break;
        }
		case DREQ:
			error("Invalid master message\n");
	}
}
/*void Node::update_offset_previous(double value){
	offset_previous = value;
	ev<<"offset_previous = "<<offset_previous<<endl;
}*/

void Slave::ProduceT3packet(){

 /*   //dxw: skip the Packet exchange with Clock for timestamping
    Packet *pck = new Packet("DREQ_TIME_REQ");
	pck->setPckType(PTP);
	pck->setSource(address);
	pck->setDestination(master);
	pck->setPtpType(DREQ);
	pck->setClockType(TIME_REQ);
	send(pck,"outclock");
*/
    // dxw: Instead, call Clock::getTimeStamp() to get the time stamp
     // ts3 = SIMTIME_DBL(msg->getTimestamp());
      ts3 = pClock->getTimestamp();
      ev<<"The time of T3:"<<endl;
      ev<<"T3="<<ts3<<endl;
      PtpPkt *pck = new PtpPkt("DREQ"); // to replace  msg->setName("DREQ");
      pck->setDestination(master);
      pck->setByteLength(DREQ_BYTE);
      // we use the host modules findHost() as a appl address
      pck->setSource(address);
      pck->setPtpType(DREQ);
      pck->setDestAddr(LAddress::L3Type(pck->getDestination()));
      pck->setSrcAddr(LAddress::L3Type(pck->getSource()));
      // set the control info to tell the network layer the layer 3
      // address;
      NetwControlInfo::setControlInfo(pck, LAddress::L3Type(pck->getDestination()));

      send((cMessage *)pck,"out");
      ev<<"slave Node sending DREQ out to its master\n";

}

void Slave::recordResult(){
	dsmVec.record(dsm);
	dpropVec.record(dprop);
	dmsVec.record(dms);
	offsetVec.record(offset);
	driftVec.record(drift);
	delta_t41Vec.record(delta_t41);
	TrVec.record(Tr);

}
void Slave::finish(){}

void Slave::updateDisplay(){
	char buf[100];
	sprintf(buf, "dms [ms]: %3.2f \ndsm [ms]: %3.2f \ndpr [ms]: %3.2f \noffset [ms]: %3.2f\n ",
		dms*1000,dsm*1000,dprop*1000,offset*1000);
	getDisplayString().setTagArg("t",0,buf);
}
/*
-------------------------------------------------------------------------------
SERVO CLOCK IMPLEMENTATION.
This function must be overwritten by the user.
-------------------------------------------------------------------------------*/

void Slave::servo_clock(){
	dms = ts2 - ts1;
	dsm = ts4 - ts3;
	ev<<"dms="<<dms<<"dsm="<<dsm<<endl;
	dprop = (dms + dsm)/2;
	offset = dms - dprop;
	//offset = ((ts2 - ts1)- (ts4 - ts3))/2;
	ev<<"offset="<<offset<<endl;
    //double alpha = 1;
    //double beta = 200;
    //double y =offset/alpha;
	//ev<<"y="<<y<<endl;
    ev<<"offset= "<<offset<<endl;

/*    // the following codes of packet exchange will be
    // replace by calling function clock2::adjtimeex() directly
    Packet *pck = new Packet("ADJ_OFFSET");
	pck->setPckType(CLOCK);
	pck->setClockType(OFFSET_ADJ);
    //pck->setData(y);
	pck->setData(offset);
	send(pck,"outclock");
*/


	Ts = ts2;//T2
	Tm = ts1;//T1
	ev<<"Ts-Ts_previous="<<Ts-Ts_previous<<endl;
	//if(Tm_previous > 0){  //去掉这个if判断语句，不然drift在第一次同步周期内没有得到校正,去掉之后对结果没什么影响
	// TODO:drift estimate
		//drift = (Ts-Ts_previous+y)/(Tm-Tm_previous)-1;//从物理意义上来说，这个计算drift的算法是错误的
	//drift = (Ts - Ts_previous + offset_previous)/(Tm-Tm_previous)-1;//仿真结果最好的一个算法
	//drift = (Ts - Ts_previous)/(Tm-Tm_previous)-1;//在我的时钟模型中，这个算法是不对的,应在clock中加上u[0][0]/Tsync
    drift = (offset-offset_previous)/(Tm-Tm_previous);//仿真结果最差，这个算法是不对的,应在在clock中加上u[0][0]/Tsync;为了对应KF参数，采用此算法
	//drift = offset/(Tm-Tm_previous);
	/*double DELTADRIFT=10E-6;
		if(drift>DELTADRIFT){drift=DELTADRIFT;}
		else if(drift<-DELTADRIFT){drift=-DELTADRIFT;}//这个判定值与物理时钟drift的初始值有关*/
	//double x= drift/beta;
		ev<<"drift="<<drift<<endl;

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
	//dxw->hyw: see my question in Clock2.cc, line 225
	pClock->adjtimex(offset, 0);
	pClock->adjtimex(drift,1);
	pClock->adj_offset_drift();

	recordResult();

    offset_previous = offset;
	Tm_previous = Tm;
	Ts_previous = Ts;


}


cModule *Slave::findHost(void)
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
