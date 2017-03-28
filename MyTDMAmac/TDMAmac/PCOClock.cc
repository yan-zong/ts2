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

#include "PCOClock.h"
#include "Constant.h"
#include "PtpPkt_m.h"

Define_Module(PCOClock);

void PCOClock::initialize()
{
    // ---------------------------------------------------------------------------
    // Initialise variable for saving output data
    // ---------------------------------------------------------------------------
    softclockVec.setName("softclock");
    softclock_t2Vec.setName("softclock_t2");
    softclock_t3Vec.setName("softclock_t3");
    noise1Vec.setName("noise1");
    noise2Vec.setName("noise2");
    noise3Vec.setName("noise3");
    driftVec.setName("drift");
    offsetVec.setName("offset");
    update_numberVec.setName("update_number");
    drift_valueVec.setName("drift_value");
    offset_valueVec.setName("offset_value");
    error_driftVec.setName("error_drift");
    error_offsetVec.setName("error_offset");
    delta_driftVec.setName("delta_drift");
    delta_offsetVec.setName("delta_offset");
    drift_adj_valueVec.setName("drift_adj_value");
    offset_adj_valueVec.setName("offset_adj_value");
    // phyclockVec.setName("phyclock");
    physicalClockVec.setName("PhysicalClock");
    adjustedthresholdvalueVec.setName("ThresholdAdjustValue");
    thresholdVec.setName("RegisterThreshold");
    thresholdOffsetVec.setName("ThresholdOffset");
    pulsetimeVec.setName("PulseTime");

    offsetTotalVec.setName("OffsetTotal");

    // ---------------------------------------------------------------------------
    // Initialise variable
    // ---------------------------------------------------------------------------
    offset = par("offset");
    drift =  par("drift");
    sim_time_limit = par("sim_time_limit");
    sigma1  = par("sigma1");
    sigma2 = par("sigma2");
    sigma3 = par("sigma3");
    u3 = par("u3");
    Tcamp  = par("Tcamp");
    Tsync = par("Tsync");
    // RegisterThreshold = par("RegisterThreshold");

    Threshold = par("RegisterThreshold");

    FrameDuration = par ("FrameDuration");
    slotDuration = par("slotDuration");
    ScheduleOffset = par("ScheduleOffset");
    delay = par("delay");
    AdjustParameter = par("AdjustParameter");
    k = int(sim_time_limit/Tcamp);

    error_drift = offset;
    error_offset = drift;
    // lastupdatetime = SIMTIME_DBL(simTime());
    // phyclock = softclock = offset;
    drift_previous = drift;
    offset_previous = offset;
    i = 0;
    j = 0;
    delta_drift = delta_offset = 0;
    Tm = Tm_previous =0;
    offset_adj_previous=0;
    PulseTime = 0;

    // ---------------------------------------------------------------------------
    // Initialise variable for PCO, when times of clock update reaches the
    // 'UpdateTimes', also means the clock time reaches the threshold
    // (RegisterThreshold). The clock time reset to zero, and a pulse is generated
    // and broadcasted at the same time
    // ---------------------------------------------------------------------------
    ev << "Clock: the threshold of register is " << Threshold << "s." << endl;
    PulseTimePrevious = 0;
    numPulse = 0;
    LastUpdateTime = SIMTIME_DBL(simTime());
    ThresholdAdjustValue = 0;
    RefTimePreviousPulse = 0;
    offsetTotal = 0;
    ReceivedPulseTime = 0;


    ReferenceClock = 0;
    PhysicalClock = 0;
    ThresholdTotal = 0;
    PCOClock = 0;

    NodeId = (findHost()->getId() - 4);
    EV << "PCOClock: the node id is " << NodeId << ", and 'ScheduleOffset+slotDuration*NodeId' is "<< ScheduleOffset+slotDuration*NodeId <<endl ;
    // id of master should be 0;
    // id of relay[0] should be 1; id of relay[1] should be 2;

    EV << "yan: findHost()->getIndex() is " << findHost()->getIndex() << endl ;
    EV << "yan: findHost()->getId() is " << findHost()->getId() << endl ;

    ClockOffset = 0;
    ThresholdOffset = 0;

    if(ev.isGUI())
    {
        updateDisplay();
    }

    delta_driftVec.record(delta_drift);
    delta_offsetVec.record(delta_offset);

    // Tcamp is clock update period
    scheduleAt(simTime() + ScheduleOffset + NodeId*slotDuration ,new cMessage("CLTimer"));
    LastUpdateTime = ScheduleOffset + NodeId*slotDuration;
    RefTimePreviousPulse = LastUpdateTime;

    EV << "PCOClock: Clock starts at " << (simTime() + NodeId*slotDuration + ScheduleOffset) << endl;
}

void PCOClock::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        physicalClockUpdate();   // update physical clock by clock offset and drift
        EV << "PCOClock: Physical Clock is " << PhysicalClock << endl;

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
double PCOClock::physicalClockUpdate()
{
    ev << "PCOClock: the PREVIOUS offset is "<< offset << endl;
    noise2 =  normal(0,sigma2,1);
    offset = offset + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)+ noise2;
    ev << "PCOClock: the UPDATED offset is "<< offset << endl;

    ev << "PCOClock: the PREVIOUS drift is "<< drift <<endl;
    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    ev << "PCOClock: the UPDATED drift is "<< drift <<endl;

    if ( NodeId == 0)   // master node
    {
        ev << "PCOClock: the PREVIOUS Reference clock is "<< ReferenceClock << endl;
        ReferenceClock = offset + SIMTIME_DBL(simTime());
        ev << "PCOClock: the UPDATED Reference clock is "<< ReferenceClock << endl;
    }
    else    // relay node
    {
        ev << "PCOClock: the PREVIOUS Reference clock is "<< ReferenceClock << endl;
        ReferenceClock = offset + SIMTIME_DBL(simTime()) - (ScheduleOffset + NodeId*slotDuration);
        ev << "PCOClock: the UPDATED Reference clock is "<< ReferenceClock << endl;
    }

    if ((PhysicalClock - Threshold) > -4e-005)
    {
        ev << "PCOClock: the PREVIOUS 'ThresholdTotal' is "<< ThresholdTotal <<endl;
        ThresholdTotal = ThresholdTotal + Threshold;
        ev << "PCOClock: the UPDATED 'ThresholdTotal' is "<< ThresholdTotal <<endl;

        ev << "PCOClock: the PREVIOUS 'PhysicalClock' is "<< PhysicalClock <<endl;
        PhysicalClock = ReferenceClock - ThresholdTotal;
        ev << "PCOClock: the UPDATED 'PhysicalClock' is "<< PhysicalClock <<endl;

        generateSYNC();
        EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;
    }
    else
    {
        ev << "PCOClock: the PREVIOUS 'PhysicalClock' is "<< PhysicalClock <<endl;
        PhysicalClock = ReferenceClock - ThresholdTotal;
        ev << "PCOClock: the UPDATED 'PhysicalClock' is "<< PhysicalClock <<endl;
    }

    LastUpdateTime = SIMTIME_DBL(simTime());
    ev << "PCOClock: the 'LastUpdateTime' is "<< SIMTIME_DBL(simTime()) <<endl;
    return PhysicalClock;
}

void PCOClock::recordResult()
{
    driftVec.record(drift);
    offsetVec.record(offset);
    update_numberVec.record(i);
    noise1Vec.record(noise1);
    noise2Vec.record(noise2);
    adjustedthresholdvalueVec.record(ThresholdAdjustValue);
    //thresholdVec.record(RegisterThreshold);
    physicalClockVec.record(PhysicalClock);
    // thresholdOffsetVec.record(ThresholdOffset);

}

/* @breif get timestamp of PCO */
double PCOClock::getPCOTimestamp()
{
    ev << "PCOClock: PCOTimestamp... " << endl;
    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    noise3 = normal(u3,sigma3);
    noise3Vec.record(noise3);

    PCOClock = PhysicalClock+ drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise3;
    ev << "PCOClock: 'PCOClock' is " << PCOClock << endl;

    return PCOClock;
}

/* @breif get timestamp of local drifting clock */
double PCOClock::getTimestamp()
{
    // noise3 = normal(u3,sigma3);
    // noise3Vec.record(noise3);
    // double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal + noise3;
    // double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal;
    // double clock = softclock + numPulse * FrameDuration + offsetTotal;
    double clock = getPCOTimestamp() + numPulse * FrameDuration;

    ev << "Yan: numPulse * FrameDuration is " << numPulse * FrameDuration <<endl;
    ev << "Yan: numPulse is " << numPulse <<endl;
    ev << "Yan: FrameDuration is " << FrameDuration <<endl;
    ev << "Yan: offsetTotal is " << offsetTotal <<endl;
    ev << "Yan: getTimestamp is " << clock <<endl;

    ev << "PCOClock: numPulse * FrameDuration = " << numPulse * FrameDuration;
    ev << ", the variable 'offsetTotal' is " << offsetTotal <<endl;
    ev << ", the returned clock time is " << clock <<endl;
    return clock;
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
        // offset,drift*1E6,lastupdatetime);
           offset,drift*1E6,LastUpdateTime);
    getDisplayString().setTagArg("t",0,buf);
}

void PCOClock::getThresholdOffset()
{
    ev << "PCOClock: adjust threshold of clock... "<< endl;

    // ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - getPCOTimestamp() - ScheduleOffset*NodeId - delay;
    // ClockOffset = ReceivedPulseTime - delay - numPulse*FrameDuration - getPCOTimestamp() - ScheduleOffset - slotDuration*NodeId;
    // ClockOffset = ReceivedPulseTime - delay - numPulse*FrameDuration - softclock - ScheduleOffset - slotDuration*NodeId;
    ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - softclock;
    ThresholdOffset = ClockOffset;
    ev << "PCOClock: the threshold offset is "<< ThresholdOffset << ", and the clock offset is " << ClockOffset << endl;

    thresholdOffsetVec.record(ThresholdOffset);
    offsetTotalVec.record(offsetTotal);

    ev << "Yan: ReceivedPulseTime is " << ReceivedPulseTime <<endl;
    ev << "Yan: delay is " << delay <<endl;
    ev << "Yan: numPulse*FrameDuration is " << numPulse*FrameDuration <<endl;
    ev << "Yan: numPulse is " << numPulse <<endl;
    ev << "Yan: FrameDuration is " << FrameDuration <<endl;
    ev << "Yan: softclock is " << softclock <<endl;
    ev << "Yan: ScheduleOffset is " << ScheduleOffset <<endl;
    ev << "Yan: slotDuration*NodeId is " << slotDuration*NodeId <<endl;
    ev << "Yan: slotDurationis " << slotDuration<<endl;
    ev << "Yan: NodeId is " << NodeId <<endl;
    ev << "Yan: ClockOffset is " << ClockOffset <<endl;

    ev << "PCOClock: threshold adjustment value is obtained "<< endl;
}


void PCOClock::adjustThreshold()
{
    // ThresholdAdjustValue = AdjustParameter*ThresholdOffset;
    ThresholdAdjustValue = ThresholdOffset;
    ev << "PCOClock: based on the threshold adjustment value: "<< ThresholdAdjustValue << ", the RegisterThreshold change from " << Threshold;
    Threshold = Threshold - ThresholdAdjustValue;
    ev << " to " << Threshold << endl;

    thresholdVec.record(Threshold);
}

int PCOClock::getnumPulse()
{
    ev << "PCOClock: the returned 'numPulse' is " << numPulse << endl;
    return numPulse;
}

void PCOClock::generateSYNC()
{
    EV << "PCOClock: time reaches threshold, generate a SYNC packet \n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    pck->setByteLength(44);
    send(pck,"outclock");

    EV << "PCOClock transmits SYNC packet to Core module" << endl;
}

double PCOClock::setReceivedTime(double value)
{
    ReceivedPulseTime = value;
    return ReceivedPulseTime;
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

