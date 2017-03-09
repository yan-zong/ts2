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
	/*Ts��ʾ��������ݰ��ڻ����������ѵ�ʱ�䣬��ʱ��ļ��㷽���ڿ��ص����ݰ���λ��λ����֮��ı���lunghessa��*/
	// ---------------------------------------------------------------------------
	ev << "BUFFER : Inizializzazione ...\n";
	queue.setName("buffer");
	pckVec.setName("pacchetti");//pacchetti,��
	byteVec.setName("byte");
	pacchetti = 0;			 // numero di pacchetti in coda  ����/��ջ������ݰ�
	byte = 0;				 // numero di byte occupati ռ�õ��ֽ���
	Ts = 0;				 // tempo impiegato per spedire un pacchetto  ����һ�����ݰ����ѵ�ʱ��
	rate=par("rate");	 // [bit/s]
	if(rate==0)
		T=0;
	else
		T=1/rate;
	Tlat=par("latenza"); // [s] latenza: impostare a zero �ж���ʱ,����Ϊ��
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
		ev << "BUFFER : Ora di invio del pacchetto = " << simTime()*1000 << " [ms]\n";//���ݰ��ĵ���ʱ��
		if(!queue.empty()){
			Ts = ((cPacket *)queue.front())->getByteLength()*8*T+Tlat+normal(0,1e-6); //may be back???
			scheduleAt(simTime()+Ts, new cMessage("BufTimer"));
		}
		delete msg;
	}
	else{
		ev << "BUFFER : Ora di arrivo del pacchetto = " << simTime()*1000 << " [ms]\n";//���ݰ��ĵ���ʱ��
		if(!queue.empty()){
			//ev<<"!queue.empty()= "<<!queue.empty()<<endl;
			//Inserisco il messaggio arrivato nel buffer.  ������Ϣ�ִﻺ����������Ϣ�ݴ���buffer�Ķ�ջ�У�����
			queue.insert(msg);
			pacchetti = pacchetti +1;
			byte = byte + check_and_cast<cPacket *>(msg)->getByteLength();//??
		}else{
			//Calcolo il tempo del servizio del messaggio arrivato.����������Ϣ��ʱ������
			ev<<"Success handle else!"<<endl;
			ev<<"receive_msg_time="<<simTime()<<endl;
			Ts = check_and_cast<cPacket *>(msg)->getByteLength()*8*T+Tlat;//??
			ev << "BUFFER : Tempo di servizio Ts = " << Ts << "\n";//����ʱ�� Ts
			//Inserisco il messaggio arrivato nel buffer.
			queue.insert(msg);
			pacchetti = pacchetti +1;
			byte = byte + check_and_cast<cPacket *>(msg)->getByteLength();//??
			//Imposto il timer per spedire il messaggio.
			//ʵ�ж�ʱ������Ϣ,�൱��ÿ��TS�Ӷ�ջ�з���ȥһ������node�ڵ㡣��TS=0����buffer�о��޴����ӳ٣���
			scheduleAt(simTime()+Ts, new cMessage("BufTimer"));
		}
	}
	if(ev.isGUI()){updateDisplay();}
	/*********************************
	 **    Salvataggio valori.   ����ֵ   **
	 *********************************/
	pckVec.record(pacchetti);
	byteVec.record(byte);
}

void Buffer::finish(){
	ev << "BUFFER : finish...\n";
	ev << "BUFFER : Rimozione di tutti i messaggi in coda...\n";//ɾ�������Ŷӵ���Ϣ
	while(!queue.empty()){
		delete (cMessage *)queue.pop();
	}
	ev << "BUFFER : Tutti i messaggi sono stati rimossi.\n";//���е���Ϣ�ѱ�ɾ��
}

void Buffer::updateDisplay(){
	char buf[40];
	sprintf(buf, "pck: %d   byte: %d\n",pacchetti,byte);
	getDisplayString().setTagArg("t",0,buf);
}
