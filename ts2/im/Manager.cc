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

#include <string.h>
#include <omnetpp.h>
#include "Packet_m.h"
#include "Event_m.h"
#include "Constant.h"

class Manager:public cSimpleModule{
protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
private:
	double Tcyc;
	int pckLength;
	int pckNumber;
	int slaveNumber;
	int *nodi;
	int n;
};

Define_Module(Manager);

void Manager::initialize(){
	nodi = new int[100];
	Tcyc = par("Tciclico"); // [s]
	pckLength = par("Byte");	//dimensione di un pacchetto in byte
	pckNumber = par("Number");	//numero di pacchetti

	/*Identificazione del nodo master*/
	int index=0;
	nodi[index] = getParentModule()->getParentModule()->findSubmodule("master");
	ev << "MANAGER : index = " << index << ", ID master = " << nodi[index] << "\n";
	index++;
	/*Identificazione dei nodi slave*/
	int islave =0;
	while(getParentModule()->getParentModule()->findSubmodule("slave",islave)!= -1){
		nodi[index]=getParentModule()->getParentModule()->getSubmodule("slave",islave)->findSubmodule("nodo");
		ev << "MANAGER : index = " << index << ", ID slave = " << nodi[index] << "\n";
		index++; 
		islave++;
	}
	if(getParentModule()->getParentModule()->findSubmodule("sink")!= -1){
		nodi[index]=getParentModule()->getParentModule()->findSubmodule("sink");
		ev << "MANAGER : index = " << index << ", ID sink = " << nodi[index] << "\n";
		index++;
	}else{
		ev << "MANAGER : non ci sono nodi sink.\n";
	}
	if(getParentModule()->getParentModule()->findSubmodule("source")!= -1){
		nodi[index]=getParentModule()->getParentModule()->findSubmodule("source");
		ev << "MANAGER : index = " << index << ", ID source = " << nodi[index] << "\n";
		index++;
	}else{
		ev << "MANAGER : non ci sono nodi source.\n";
	}
	n=index;
	if(Tcyc>0)
		scheduleAt(simTime()+Tcyc, new cMessage("Etimer"));
}

void Manager::handleMessage(cMessage *msg){
	Event *evt = new Event("EVENT_MSG");
	evt->setEventType(CICLICO);
	evt->setPckLength(pckLength);
	evt->setPckNumber(pckNumber);
	evt->setDest(nodi[0]);
	send(evt,"out");
	scheduleAt(simTime()+Tcyc, new cMessage("Etimer"));
	delete msg;
}
void Manager::finish(){
	ev << "MANAGER : Finish...\n";
	ev << "MANAGER : Elimino il vettore nodi.\n";
	delete nodi;
	ev << "MANAGER : Eliminazione completata.\n";
}