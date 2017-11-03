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

#include "PCOClock.h"
#include "Constant.h"
#include "PtpPkt_m.h"

Define_Module(PCOClock);

void PCOClock::initialize()
{
    // ---------------------------------------------------------------------------
    // Initialise variable for saving output data
    // ---------------------------------------------------------------------------
    driftVec.setName("drift");
    offsetVec.setName("offset");
    noise1Vec.setName("noise1");
    noise2Vec.setName("noise2");
    noise3Vec.setName("noise3");
    pcoclockVec.setName("PCOClockState");
    classicclockVec.setName("ClassicClock");
    thresholdVec.setName("Threshold");
    update_numberVec.setName("update_number");
    measurementoffsetVec.setName("MeasurementOffset");

    // ---------------------------------------------------------------------------
    // Initialise variable
    // ---------------------------------------------------------------------------
    offset = par("offset"); // the clock offset
    drift =  par("drift");  // the clock skew (the variation of clock frequency)
    sim_time_limit = par("sim_time_limit"); // simulation time
    sigma1  = par("sigma1");    // the standard deviation of clock skew noise
    sigma2 = par("sigma2"); // the standard deviation of clock offset noise
    sigma3 = par("sigma3"); // the standard deviation of timestamp (meausmrenet) noise
    u3 = par("u3"); // the mean of timestamp (measurement) noise
    Tcamp  = par("Tcamp");  // clock update period
    Threshold = par("Threshold");
    pulseDuration = par("pulseDuration");
    ScheduleOffset = par("ScheduleOffset");
    tau = par("tau");   // the transmission delay

    ReferenceClock = 0;
    ClassicClock = 0;
    PCOClockState = 0;
    ThresholdTotal = 0;
    Timestamp = 0;   // timestamp based on the reception of SYNC packet
    LastUpdateTime = SIMTIME_DBL(simTime());
    MeasurementOffset = 0;
    ReceivedSYNCTime = 0;   // the reception time of SYNC packet

    k = int(sim_time_limit / Tcamp);
    drift_previous = drift;
    offset_previous = offset;
    i = 0;
    j = 0;
    delta_drift = delta_offset = 0;
    Tm = Tm_previous =0;
    offset_adj_previous=0;

    NodeId = (findHost()->getId() - 4);
    EV << "PCOClock: the node id is " << NodeId << ", and 'ScheduleOffset + pulseDuration * NodeId' is "<< ScheduleOffset + pulseDuration * NodeId <<endl ;
    // id of master should be 0;
    // id of relay[0] should be 1; id of relay[1] should be 2;

    // EV << "yan: findHost()->getIndex() is " << findHost()->getIndex() << endl ;
    // EV << "yan: findHost()->getId() is " << findHost()->getId() << endl ;

    if(ev.isGUI())
    {
        updateDisplay();
    }

    delta_driftVec.record(delta_drift);
    delta_offsetVec.record(delta_offset);

    scheduleAt(simTime() + ScheduleOffset + NodeId*pulseDuration ,new cMessage("CLTimer"));
    LastUpdateTime = ScheduleOffset + NodeId*pulseDuration;

    EV << "PCOClock: Clock starts at " << (simTime() + ScheduleOffset + NodeId * pulseDuration) << endl;
}

void PCOClock::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        ClockUpdate();   // update PCO clock
        EV << "PCOClock: PCO Clock State is " << PCOClockState << endl;

        i = i + 1;
        ev << "i = "<< i << endl;
        ev << "k = " << k << endl;

        if(i % 10 == 0)
        {
            ev << "count delta_drfit and delta_offset" << endl;
            delta_drift = drift - drift_previous;
            delta_offset = offset - offset_previous;
            if(k >= 9999999)
            {
                ev << "compare 1 success!"<<endl;
                if(i % 100 == 0)
                {
                    ev << "Larger amount of data, record delta_offset and delta_drift."<<endl;
                    delta_driftVec.record(delta_drift);
                    delta_offsetVec.record(delta_offset);
                }
            }
            else
            {
                ev << "less amount of data, record delta_offset and delta_drift."<<endl;
                delta_driftVec.record(delta_drift);
                delta_offsetVec.record(delta_offset);
            }

            drift_previous = drift;
            offset_previous = offset;
        }

        if(k >= 9999999)
        {
            ev << "compare 2 success!"<<endl;
            if((i > 10) && (i % 100 == 0))
            {
                ev << "Larger amount of data, record offset and drift of updatePhyclock"<<endl;
                recordResult();
                driftStd.collect(drift);
                offsetStd.collect(offset);
            }
        }
        else
        {
            ev << "Less amount of data,record offset and drift of updatePhyclock"<<endl;
            recordResult();
            driftStd.collect(drift);
            offsetStd.collect(offset);
        }

        scheduleAt(simTime()+ Tcamp,new cMessage("CLTimer"));
    }

    else
    {
        error("PCOClock receipts a Packet from node. It does not exchange Packet with node.\n");
    }

    delete msg;

    if(ev.isGUI())
    {
        updateDisplay();
    }
}

/* when the clock time reach the threshold value, the clock time will be reset to zero */
double PCOClock::ClockUpdate()
{
    ev << "PCOClock: the PREVIOUS offset is "<< offset << endl;
    noise2 =  normal(0,sigma2,1);
    offset = offset + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise2;
    ev << "PCOClock: the UPDATED offset is "<< offset << endl;

    ev << "PCOClock: the PREVIOUS drift is "<< drift <<endl;
    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    ev << "PCOClock: the UPDATED drift is "<< drift <<endl;

    if ( NodeId == 0)   // master node
    {
        ev << "PCOClock: the PREVIOUS Classic clock is "<< ClassicClock << endl;
        ClassicClock = offset + SIMTIME_DBL(simTime());
        ev << "PCOClock: the UPDATED Classic clock is "<< ClassicClock << endl;
    }
    else    // relay node
    {
        ev << "PCOClock: the PREVIOUS classic clock is "<< ClassicClock << endl;
        ClassicClock = offset + SIMTIME_DBL(simTime()) - (ScheduleOffset + NodeId * pulseDuration);
        ev << "PCOClock: the UPDATED classic clock is "<< ClassicClock << endl;
    }

    if ((PCOClockState - Threshold + Tcamp + Tcamp) > 0 )
    {
        ev << "PCOClock: the PREVIOUS 'ThresholdTotal' is "<< ThresholdTotal <<endl;
        ThresholdTotal = ThresholdTotal + Threshold;
        ev << "PCOClock: the UPDATED 'ThresholdTotal' is "<< ThresholdTotal <<endl;

        ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
        PCOClockState = ClassicClock - ThresholdTotal;
        ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;

        generateSYNC();
        EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

    }
    else
    {
        ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
        PCOClockState = ClassicClock - ThresholdTotal;
        ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
    }

    LastUpdateTime = SIMTIME_DBL(simTime());
    ev << "PCOClock: the 'LastUpdateTime' is "<< SIMTIME_DBL(simTime()) <<endl;

    pcoclockVec.record(PCOClockState);
    classicclockVec.record(ClassicClock);

    return PCOClockState;
}

void PCOClock::recordResult()
{
    offsetVec.record(offset);
    driftVec.record(drift);
    noise1Vec.record(noise1);
    noise2Vec.record(noise2);
    update_numberVec.record(i);
}

/* @breif generate timestamp */
double PCOClock::getTimestamp()
{
    ev << "PCOClock: PCOTimestamp... " << endl;
    noise3 = normal(u3,sigma3);
    noise3Vec.record(noise3);

    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    Timestamp = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise3;
    ev << "PCOClock: 'Timestamp' is " << Timestamp << endl;

    return Timestamp;
}

void PCOClock::finish()
{
    error_sync_drift.getMin();
    error_sync_drift.getMax();
    error_sync_offset.getMin();
    error_sync_offset.getMax();

    recordScalar("sigma3",sigma3);
    recordScalar("drift_mean",driftStd.getMean());
    recordScalar("drift_std",driftStd.getStddev());
    recordScalar("offset_mean",offsetStd.getMean());
    recordScalar("offset_std",offsetStd.getStddev());
    recordScalar("error_drift_mean",error_sync_drift.getMean());
    recordScalar("error_drift_std",error_sync_drift.getStddev());
    recordScalar("error_offset_mean",error_sync_offset.getMean());
    recordScalar("error_offset_std",error_sync_offset.getStddev());
}


void PCOClock::updateDisplay()
{
    char buf[100];
    sprintf(buf, "offset [msec]: %3.2f   \ndrift [ppm]: %3.2f \norigine: %3.2f",
           offset,drift*1E6,LastUpdateTime);
    getDisplayString().setTagArg("t",0,buf);
}

double PCOClock::getMeasurementOffset(int MeasurmentAlgorithm, int AddressOffset)
{
    ev << "PCOClock: get measurement offset... "<< endl;

    // Note: 2.176E-3 (tau) means the time for receiver to recept the SYNC packet from the sender,
    // 1.82E-4 is for physical layer to check the SYNC packet
    // 2.176E-3 = 2.368E-3 - 1.82E-4

    // the 'getSource()' function of packet can be used to determine where is the received SYNC from

    if (MeasurmentAlgorithm == 1)   // the receipted SYNC is from master node
        MeasurementOffset = (ReceivedSYNCTime - tau) - Threshold + (ScheduleOffset + pulseDuration * NodeId);
    else if (MeasurmentAlgorithm == 2)  // node i receives the SYNC from node j (node i fires before node j)
        MeasurementOffset = (ReceivedSYNCTime - tau) - 0 - (pulseDuration * AddressOffset);
    else if (MeasurmentAlgorithm == 3)  // node j receives the SYNC from node i (node i fires before node j)
        MeasurementOffset = (ReceivedSYNCTime - tau) - Threshold + (pulseDuration * AddressOffset);

    ev << "PCOClock: 'MeasurementOffset' is " << MeasurementOffset << endl;

    if (MeasurmentAlgorithm == 1)
        measurementoffsetVec.record(MeasurementOffset);
    else if (MeasurmentAlgorithm == 2)
        measurementoffsetVec.record(MeasurementOffset);
    else if (MeasurmentAlgorithm == 3)
        measurementoffsetVec.record(MeasurementOffset);

    return MeasurementOffset;
}

void PCOClock::generateSYNC()
{
    EV << "PCOClock: PCO clock state reaches threshold, it is reset to zero, meanwhile, a SYNC packet is generated \n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    pck->setByteLength(44);
    send(pck,"outclock");

    EV << "PCOClock transmits SYNC packet to Core module" << endl;
}

double PCOClock::setReceivedSYNCTime(double value)
{
    ReceivedSYNCTime = value;
    return ReceivedSYNCTime;
}

cModule *PCOClock::findHost(void)
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

double PCOClock::AbsoluteValue(double AbsoluteValueInput)
{
    double AbsoluteValueOutput;
    if (AbsoluteValueInput > 0 )
        AbsoluteValueOutput = AbsoluteValueInput;
    else
        AbsoluteValueOutput = 0 - AbsoluteValueInput;

    return AbsoluteValueOutput;
}


