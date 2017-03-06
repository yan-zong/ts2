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
    phyclockVec.setName("phyclock");
    adjustedthresholdvalueVec.setName("ThresholdAdjustValue");
    thresholdVec.setName("RegisterThreshold");
    thresholdOffsetVec.setName("ThresholdOffset");
    pulsetimeVec.setName("PulseTime");

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
    RegisterThreshold = par("RegisterThreshold");
    FrameDuration = par ("FrameDuration");
    slotDuration = par("slotDuration");
    ScheduleOffset = par("ScheduleOffset");
    delay = par("delay");
    AdjustParameter = par("AdjustParameter");
    k = int(sim_time_limit/Tcamp);

    error_drift = offset;
    error_offset = drift;
    lastupdatetime = SIMTIME_DBL(simTime());
    phyclock = softclock = offset;
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
    ev << "Clock: the threshold of register is " << RegisterThreshold << "s." << endl;
    PulseTimePrevious = 0;
    numPulse = 0;
    LastUpdateTime = SIMTIME_DBL(simTime());
    ThresholdAdjustValue = 0;
    RefTimePreviousPulse = 0;
    offsetTotal = 0;
    ReceivedPulseTime = 0;

    NodeId = (findHost()->getIndex() + 1);
    EV << "PCOClock: the node id is " << NodeId << ", and 'ScheduleOffset+slotDuration*NodeId' is "<< ScheduleOffset+slotDuration*NodeId <<endl ;
    // id of relay[0] should be 1; id of relay[1] should be 2;

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
        Phyclockupdate();   // update physical clock by clock offset and drift
        EV << "PCOClock: phyclock = " << phyclock << endl;

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
double PCOClock::Phyclockupdate()
{
    ev << "PCOClock: update clock, the PREVIOUS offset is "<< offset << ", and PREVIOUS drift is "<< drift <<endl;

    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    ev << "PCOClock: the UPDATED drift is "<< drift <<endl;

    noise2 =  normal(0,sigma2,1);
    offset = offset + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)+ noise2;
    ev << "PCOClock: the UPDATED offset is "<< offset << endl;

    ev << "PCOClock: the PREVIOUS physical clock time is " << phyclock << endl;
    ev << "PCOClock: phyclock - RegisterThreshold = " << (phyclock - RegisterThreshold) << endl;

    // if (phyclock - (RegisterThreshold - Tcamp) > (-1E-6))
    // if (((phyclock - RegisterThreshold) > (-30.51757813E-6)) | ((phyclock - RegisterThreshold) == (-30.51757813E-6)))
    if ((phyclock - RegisterThreshold) > -4e-005)
    {
        numPulse = numPulse + 1;
        RefTimePreviousPulse = SIMTIME_DBL(simTime());
        offsetTotal = offsetTotal + offset; // record the offset
        EV << "PCOClock: the 'numPulse' is "<< numPulse <<endl;
        EV << "PCOClock: the 'RefTimePreviousPulse' is "<< RefTimePreviousPulse <<endl;
        EV << "PCOClock: the 'offsetTotal' is "<< offsetTotal <<endl;

        PulseTime = SIMTIME_DBL(simTime());
        pulsetimeVec.record(PulseTime);

        phyclock = 0;
        offset = 0;
        EV << "Clock: because the clock time is greater than threshold value " << RegisterThreshold;
        EV << ", the clock time is RESET to " << phyclock;
        EV << ", and the offset is also RESET to " << offset << endl;

        generateSYNC();
        EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

    }
    else
    {
        phyclock = offset + (SIMTIME_DBL(simTime()) - RefTimePreviousPulse);
        ev << "PCOClock: based on the UPDATED offset: "<< offset << ", the UPDATED physical clock time is " << phyclock << endl;
    }

    LastUpdateTime = SIMTIME_DBL(simTime());
    ev << "PCOClock: the variable 'LastUpdateTime' is "<< SIMTIME_DBL(simTime()) <<endl;
    return phyclock;
}

void PCOClock::recordResult()
{
    driftVec.record(drift);
    offsetVec.record(offset);
    update_numberVec.record(i);
    noise1Vec.record(noise1);
    noise2Vec.record(noise2);
    adjustedthresholdvalueVec.record(ThresholdAdjustValue);
    thresholdVec.record(RegisterThreshold);
    phyclockVec.record(phyclock);
    thresholdOffsetVec.record(ThresholdOffset);

}

/* @breif get timestamp of PCO */
double PCOClock::getPCOTimestamp()
{
    ev << "PCOClock: Timestamp... " << endl;
    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    double PCOClock = phyclock + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime);
    ev << "PCOClock = " << PCOClock << endl;

    softclock = PCOClock;
    ev << "softclock = " << softclock << endl;

    softclockVec.record(softclock);
    ev << "PCOClock: the returned Clock time is " << softclock <<endl;
    return softclock;
}

/* @breif get timestamp of local drifting clock */
double PCOClock::getTimestamp()
{
    noise3 = normal(u3,sigma3);
    noise3Vec.record(noise3);
    // double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal + noise3;
    double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal;

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
        offset,drift*1E6,lastupdatetime);
    getDisplayString().setTagArg("t",0,buf);
}

void PCOClock::adjustThreshold()
{
    ev << "PCOClock: adjust threshold of clock... "<< endl;

    // ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - getPCOTimestamp() - ScheduleOffset*NodeId - delay;
    ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - getPCOTimestamp() - delay - ScheduleOffset - slotDuration*NodeId;
    ThresholdOffset = ClockOffset;
    ev << "PCOClock: the threshold offset is "<< ThresholdOffset << ", and the clock offset is " << ClockOffset << endl;

    ThresholdAdjustValue = AdjustParameter*ThresholdOffset;
    ev << "PCOClock: based on the threshold adjustment value: "<< ThresholdAdjustValue << ", the RegisterThreshold change from " << RegisterThreshold;
    RegisterThreshold = RegisterThreshold - ThresholdAdjustValue;
    ev << " to " << RegisterThreshold << endl;
    ev << "PCOClock: adjust threshold of clock is finished "<< endl;
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
    pck->setByteLength(40);
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

