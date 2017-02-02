/*
Copyright(C) 2007 Giada Giorgi

This file is part of X-Simulator.

    X-Simulator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    X-Simulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Nodo.h"

Define_Module(Nodo);

void Nodo::initialize(){

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
	ts_s_sync = ts_m_sync =	ts_s_dreq =	ts_m_dreq = 0;
	dprop  = dms = dsm  = 0;
	drift = 0;
	if(ev.isGUI()){updateDisplay();}
	/* Lettura dei parametri di ingresso. */
	//Tsync = par("Tsync");
	//Tcamp  = par("Tcamp");
	/*Parametri servo clock*/
	Ts = Ts_correct = Tm = Tm_previous = Ts_previous = 0;
	offset_previous=0;

	/* Nome e Id del nodo. */
	name = "slave";
	address = findHost()->getId(); // get the node's ID, not submodule's ID

	/* Identificazione del master. */
	cModule *masterModule = findHost()->getParentModule();
	ev<<"findHost()->getParentModule returns: "<<masterModule->getName()<<endl;
	masterModule=masterModule->getSubmodule("mnode");
	if (masterModule==NULL)
	{
		error("No master node is found.\n");
	}
	master = masterModule->getId();
	myMasterNode=masterModule;

	/* Registrazione nodo nella rete. */
	Packet * temp = new Packet("REGISTER");
	temp->setPckType(OTHER);
	temp->setSource(address);
	temp->setDestination(-2);
	temp->setByteLength(0);
	send(temp,"out");
	ev << "SLAVE " << getId() << " : Fine inizializzazione.\n";
	//recordResult();
}


void Nodo::handleMessage(cMessage *msg){
	if(msg->isSelfMessage()){handleSelfMessage(msg);}
	if(msg->arrivedOn("inclock")){handleClockMessage(msg);}
	if(msg->arrivedOn("in"))
		//Un messaggio arrivato da questa porta ? sicuramente un pacchetto
		if(((Packet*)msg)->getSource()!=address &
			(((Packet*)msg)->getDestination()==address |
			((Packet*)msg)->getDestination()==-1))
			if((((Packet *)msg)->getPckType())==PTP)
				handleMasterMessage(msg);
	if(msg->arrivedOn("inevent")){handleEventMessage(msg);}
	delete msg;
	if(ev.isGUI()){updateDisplay();}
}
//ToDo Modify handleSelfMessage()
void Nodo::handleSelfMessage(cMessage *msg){

	           ProduceT3packet();

}
void Nodo::handleOtherPacket(cMessage *msg){}

void Nodo::handleEventMessage(cMessage *msg){
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


void Nodo::handleClockMessage(cMessage *msg){
	switch(((Packet *)msg)->getPtpType()){
		case SYNC:
			ts_s_sync =SIMTIME_DBL(msg->getTimestamp());
			ev<<"The time of T2:"<<endl;
            ev<<"T2="<<ts_s_sync<<endl;
            T= 1/rate;
            Tr= ((Packet *)msg)->getByteLength()*8; //包的传输时延
           // delay = uniform(0,Tcamp/10);//t2包和t3包间的处理时延
            delay = uniform(0,1E-5);
            //delay=1E-5;
            scheduleAt(simTime()+delay, new cMessage("SLtimer"));
            delayVec.record(delay);//记录t2,t3间的时延
            ev<<"delay= "<<delay<<endl;
           	break;
		//  scheduleAt(simTime()+uniform(0,10*Tcamp), new cMessage("SLtimer"));
            //scheduleAt(simTime()+uniform(0,1E-4), new cMessage("SLtimer"));
		case DREQ:
			ts_s_dreq = SIMTIME_DBL(msg->getTimestamp());
			ev<<"The time of T3:"<<endl;
            ev<<"T3="<<ts_s_dreq<<endl;
			msg->setName("DREQ");
			((Packet*)msg)->setDestination(master);
			((Packet*)msg)->setByteLength(DREQ_BYTE);
			send((cMessage *)msg->dup(),"out");
			break;
		case DRES:
			error("Invalid clock message\n");
			break;
	}
	//TODO: update offset_previous
	/*if((((Packet *)msg)->getClockType()==PROCESSED_OFFSET)){
		double a =  ((Packet *)msg)->getData();
		ev<<"a = "<<a<<endl;
	}*/
}

void Nodo::handleMasterMessage(cMessage *msg){
	switch(((Packet *)msg)->getPtpType()){
		case SYNC:
			ts_m_sync = ((Packet *)msg)->getData();
			ev<<"The time of T1:"<<endl;
            ev<<"T1="<<ts_m_sync<<endl;
            ev<<"The arival time of SYNC="<<SIMTIME_DBL(simTime())<<endl;
			msg->setName("SYN_TIME_REQ");
			((Packet *)msg)->setClockType(TIME_REQ);
			send((cMessage *)msg->dup(),"outclock");
			break;
		case DRES:
			ev<<"Nodo handleMasterMesage processes DRES Packet from Master.\n";
			ts_m_dreq = ((Packet *)msg)->getData();
			ev<<"The time of T4:"<<endl;
            ev<<"T4="<<ts_m_dreq<<endl;
            delta_t41 = SIMTIME_DBL(simTime()) - ts_m_sync;
			servo_clock();
			break;
		case DREQ:
			error("Invalid master message\n");
	}
}
/*void Nodo::update_offset_previous(double value){
	offset_previous = value;
	ev<<"offset_previous = "<<offset_previous<<endl;
}*/

void Nodo::ProduceT3packet(){
	Packet *pck = new Packet("DREQ_TIME_REQ");
	pck->setPckType(PTP);
	pck->setSource(address);
	pck->setDestination(master);
	pck->setPtpType(DREQ);
	pck->setClockType(TIME_REQ);
	send(pck,"outclock");
}
void Nodo::recordResult(){
	dsmVec.record(dsm);
	dpropVec.record(dprop);
	dmsVec.record(dms);
	offsetVec.record(offset);
	driftVec.record(drift);
	delta_t41Vec.record(delta_t41);
	TrVec.record(Tr);

}
void Nodo::finish(){}

void Nodo::updateDisplay(){
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

void Nodo::servo_clock(){
	dms = ts_s_sync - ts_m_sync;
	dsm = ts_m_dreq - ts_s_dreq;
	ev<<"dms="<<dms<<"dsm="<<dsm<<endl;
	dprop = (dms + dsm)/2;
	offset = dms - dprop;
	//offset = ((ts_s_sync - ts_m_sync)- (ts_m_dreq - ts_s_dreq))/2;
	ev<<"offset="<<offset<<endl;
    //double alpha = 1;
    //double beta = 200;
    //double y =offset/alpha;
	//ev<<"y="<<y<<endl;
    ev<<"offset= "<<offset<<endl;
	Packet *pck = new Packet("ADJ_OFFSET");
	pck->setPckType(CLOCK);
	pck->setClockType(OFFSET_ADJ);
    //pck->setData(y);
	pck->setData(offset);
	send(pck,"outclock");



	Ts = ts_s_sync;//T2
	Tm = ts_m_sync;//T1
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
		Packet *pckd = new Packet("ADJ_FREQ");
		pckd->setPckType(CLOCK);
		pckd->setClockType(FREQ_ADJ);
		//pckd->setData(x);
		pckd->setData(drift);
		ev<<"drift="<<drift<<endl;
		send(pckd,"outclock");
	//}

	recordResult();

    offset_previous = offset;
	Tm_previous = Tm;
	Ts_previous = Ts;


}


cModule *Nodo::findHost(void)
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
