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

/*
-------------------------------------------------------------------------------
BUFFER
-------------------------------------------------------------------------------
*/
class Buffer:public cSimpleModule{
protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
	virtual void updateDisplay();
private:
	cQueue queue;
	cPacket *tmp;
	int pacchetti;
	int byte;
	double rate;
	double T;
	double Tlat;
	double Ts;
	cOutVector pckVec;
	cOutVector byteVec;
};

Define_Module(Buffer);

void Buffer::initialize(){
	// ---------------------------------------------------------------------------
	// Ts rappresenta il tempo che impiega il buffer per trasmettere il pacchetto,
	//    questo tempo  viene calcolato come rapporto tra la lunghessa in bit del
	//    pacchetto e bit rate dello switch.
	/*Ts表示传输的数据包在缓冲区所花费的时间，这时间的计算方法在开关的数据包的位和位速率之间的比率lunghessa。*/
	// ---------------------------------------------------------------------------
	ev << "BUFFER : Inizializzazione ...\n";
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
	ev << "BUFFER : Fine inizializzazione.\n";	
}

void Buffer::handleMessage(cMessage *msg){
	// cPacket *msg = check_and_cast<cPacket *>(msg0);

	if(msg->isSelfMessage()){
		ev << "Buffer3 Handle Message 1.\n";
		tmp = check_and_cast<cPacket *>(queue.pop());
		pacchetti = pacchetti -1;
		byte = byte - tmp->getByteLength();
		send(tmp,"out");
		ev<<"send_msg_time="<<simTime()<<endl;
		ev << "BUFFER : Ora di invio del pacchetto = " << simTime()*1000 << " [ms]\n";//数据包的到达时间
		if(!queue.empty()){
			Ts = ((cPacket *)queue.front())->getByteLength()*8*T+Tlat+normal(0,1e-6); //may be back???
			scheduleAt(simTime()+Ts, new cMessage("BufTimer"));
		}
		delete msg;
	}
	else{
		ev << "BUFFER : Ora di arrivo del pacchetto = " << simTime()*1000 << " [ms]\n";//数据包的到达时间
		if(!queue.empty()){
			//ev<<"!queue.empty()= "<<!queue.empty()<<endl;
			//Inserisco il messaggio arrivato nel buffer.  插入消息抵达缓冲区（把消息暂存在buffer的堆栈中？？）
			queue.insert(msg);
			pacchetti = pacchetti +1;
			byte = byte + check_and_cast<cPacket *>(msg)->getByteLength();//??
		}else{
			//Calcolo il tempo del servizio del messaggio arrivato.计算服务的消息的时候到来了
			ev<<"Success handle else!"<<endl;
			ev<<"receive_msg_time="<<simTime()<<endl;
			Ts = check_and_cast<cPacket *>(msg)->getByteLength()*8*T+Tlat;//??
			ev << "BUFFER : Tempo di servizio Ts = " << Ts << "\n";//服务时间 Ts
			//Inserisco il messaggio arrivato nel buffer.
			queue.insert(msg);
			pacchetti = pacchetti +1;
			byte = byte + check_and_cast<cPacket *>(msg)->getByteLength();//??
			//Imposto il timer per spedire il messaggio.
			//实行定时发送消息,相当于每隔TS从堆栈中发出去一个包给node节点。若TS=0，那buffer中就无传输延迟？？
			scheduleAt(simTime()+Ts, new cMessage("BufTimer"));
		}
	}
	if(ev.isGUI()){updateDisplay();}
	/*********************************
	 **    Salvataggio valori.   保存值   **
	 *********************************/
	pckVec.record(pacchetti);
	byteVec.record(byte);
}

void Buffer::finish(){
	ev << "BUFFER : finish...\n";
	ev << "BUFFER : Rimozione di tutti i messaggi in coda...\n";//删除所有排队的消息
	while(!queue.empty()){
		delete (cMessage *)queue.pop();
	}
	ev << "BUFFER : Tutti i messaggi sono stati rimossi.\n";//所有的消息已被删除
}

void Buffer::updateDisplay(){
	char buf[40];
	sprintf(buf, "pck: %d   byte: %d\n",pacchetti,byte);
	getDisplayString().setTagArg("t",0,buf);
}
