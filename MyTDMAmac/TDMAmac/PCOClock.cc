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
    // thresholdOffsetVec.setName("ThresholdOffset");
    pulsetimeVec.setName("PulseTime");

    thresholdOffsetWithMasterVec.setName("ThresholdOffsetBasedMaster");
    thresholdOffsetWithrelayVec.setName("ThresholdOffsetBasedRelay");

    thresholdOffsetWithMasterIIRVec.setName("ThresholdOffsetBasedMasterIIR");
    thresholdOffsetWithrelayIIRVec.setName("ThresholdOffsetBasedRelayIIR");

    // offsetTotalVec.setName("OffsetTotal");

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

    Threshold = par("RegisterThreshold");

    FrameDuration = par ("FrameDuration");
    slotDuration = par("slotDuration");
    ScheduleOffset = par("ScheduleOffset");
    delay = par("delay");
    AdjustParameter = par("AdjustParameter");
    k = int(sim_time_limit/Tcamp);

    error_drift = offset;
    error_offset = drift;
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
    RefTimePreviousPulse = 0;
    offsetTotal = 0;
    ReceivedPulseTime = 0;


    ReferenceClock = 0;
    PhysicalClock = 0;
    ThresholdTotal = 0;
    PCOClock = 0;

    ThresholdAdjustValueBasedMasterIIR = 0;
    ThresholdAdjustValueBasedRelayIIR = 0;

    // the parameter of numerator of IIR filter
    IIRnum1 = par("IIRnum1");
    IIRnum2 = par("IIRnum2");
    IIRnum3 = par("IIRnum3");
    IIRnum4 = par("IIRnum4");
    IIRnum5 = par("IIRnum5");
    // the parameter of denominator of IIR filter
    IIRden1 = par("IIRden1");
    IIRden2 = par("IIRden2");
    IIRden3 = par("IIRden3");
    IIRden4 = par("IIRden4");
    IIRden5 = par("IIRden5");

    EV << "PCOClock: the numerator parameter of IIR Filter is "<<endl;
    EV << "IIRnum1: " << IIRnum1 << " IIRnum2: " << IIRnum2 << " IIRnum3: " << IIRnum3 << " IIRnum4: " << IIRnum4 << " IIRnum5: " << IIRnum5 << endl;

    EV << "PCOClock: the denominator parameter of IIR Filter is "<<endl;
    EV << "IIRden1: " << IIRden1 << " IIRden2: " << IIRden2 << " IIRden3: " << IIRden3 << " IIRden4: " << IIRden4 << " IIRden5: " << IIRden5 << endl;

    IIRInputMaster1 = 0;
    IIRInputMaster2 = 0;
    IIRInputMaster3 = 0;
    IIRInputMaster4 = 0;
    IIRInputMaster5 = 0;
    IIROutputMaster1 = 0;
    IIROutputMaster2 = 0;
    IIROutputMaster3 = 0;
    IIROutputMaster4 = 0;
    IIROutputMaster5 = 0;

    IIRInputRelay1 = 0;
    IIRInputRelay2 = 0;
    IIRInputRelay3 = 0;
    IIRInputRelay4 = 0;
    IIRInputRelay5 = 0;
    IIROutputRelay1 = 0;
    IIROutputRelay2 = 0;
    IIROutputRelay3 = 0;
    IIROutputRelay4 = 0;
    IIROutputRelay5 = 0;

    IIRFilterOutputMaster = 0;
    IIRFilterOutputRelay = 0;

    NodeId = (findHost()->getId() - 4);
    EV << "PCOClock: the node id is " << NodeId << ", and 'ScheduleOffset+slotDuration*NodeId' is "<< ScheduleOffset+slotDuration*NodeId <<endl ;
    // id of master should be 0;
    // id of relay[0] should be 1; id of relay[1] should be 2;

    EV << "yan: findHost()->getIndex() is " << findHost()->getIndex() << endl ;
    EV << "yan: findHost()->getId() is " << findHost()->getId() << endl ;

    ThresholdOffsetBasedMaster = 0;
    ThresholdOffsetBasedRelay = 0;

    ThresholdAdjustValueBasedMaster = 0;
    ThresholdAdjustValueBasedRelay = 0;

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

    if ((PhysicalClock - Threshold + Tcamp + Tcamp) > 0 )
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
    adjustedthresholdvalueVec.record(ThresholdAdjustValueBasedMaster);
    adjustedthresholdvalueVec.record(ThresholdAdjustValueBasedRelay);
    physicalClockVec.record(PhysicalClock);

}

/* @breif get timestamp of local drifting clock */
double PCOClock::getTimestamp()
{
    ev << "PCOClock: PCOTimestamp... " << endl;
    noise3 = normal(u3,sigma3);
    noise3Vec.record(noise3);

    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    // PCOClock = PhysicalClock+ drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise3;
    PCOClock = PhysicalClock+ drift * (SIMTIME_DBL(simTime()) - LastUpdateTime);
    ev << "PCOClock: 'PCOClock' is " << PCOClock << endl;

    return PCOClock;
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

double PCOClock::getThresholdOffsetWithMaster()
{
    ev << "PCOClock: get threshold offset of clock with master node... "<< endl;

    // Note: 2.176E-3 means the time for receiver to recept the SYNC packet from the sender,
    // 1.82E-4 is for physical layer to check the SYNC packet
    // 2.176E-3 = 2.368E-3 - 1.82E-4

    ThresholdOffsetBasedMaster = (ReceivedPulseTime  - 2.176E-3) - Threshold + ScheduleOffset + slotDuration*NodeId;
    thresholdOffsetWithMasterVec.record(ThresholdOffsetBasedMaster);
    return ThresholdOffsetBasedMaster;
}

void PCOClock::adjustThresholdBasedMaster()
{
    ThresholdAdjustValueBasedMaster = AdjustParameter * ThresholdOffsetBasedMaster;

    ThresholdAdjustValueBasedMasterIIR = IIRFilterMaster(ThresholdOffsetBasedMaster);
    thresholdOffsetWithMasterIIRVec.record(ThresholdAdjustValueBasedMasterIIR);

    ev << "PCOClock: based on the threshold adjustment value: "<< ThresholdAdjustValueBasedMaster << ", the RegisterThreshold change from " << Threshold;
    Threshold = Threshold + ThresholdAdjustValueBasedMaster;
    ev << " to " << Threshold << endl;

    thresholdVec.record(Threshold);
}

double PCOClock::getThresholdOffsetWithRelay()
{
    ev << "PCOClock: get threshold offset of clock with relay node... "<< endl;

    if (NodeId == 1)
    {
        ThresholdOffsetBasedRelay = (ReceivedPulseTime  - 2.176E-3) - 0 - slotDuration ;
        thresholdOffsetWithrelayVec.record(ThresholdOffsetBasedRelay);
    }

    if (NodeId == 2)
    {
        ThresholdOffsetBasedRelay = (ReceivedPulseTime  - 2.176E-3) - Threshold + slotDuration;
        thresholdOffsetWithrelayVec.record(ThresholdOffsetBasedRelay);
    }
    return ThresholdOffsetBasedRelay;

}

void PCOClock::adjustThresholdBasedRelay()
{
    ThresholdAdjustValueBasedRelay = AdjustParameter * ThresholdOffsetBasedRelay;

    ThresholdAdjustValueBasedRelayIIR = IIRFilterRelay(ThresholdOffsetBasedRelay);
    thresholdOffsetWithrelayIIRVec.record(ThresholdAdjustValueBasedRelayIIR);

    ev << "PCOClock: based on the threshold adjustment value: "<< ThresholdAdjustValueBasedRelay << ", the RegisterThreshold change from " << Threshold;
    Threshold = Threshold + ThresholdAdjustValueBasedRelay;
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

double PCOClock::IIRFilterMaster(double IIRFilterInput)
{
    EV << "IIRFilterMaster..." << endl;
    EV << "PREVIOUS IIRInput1: " << IIRInputMaster1 << " PREVIOUS IIRInput2: " << IIRInputMaster2 << " PREVIOUS IIRInput3: " << IIRInputMaster3 << " PREVIOUS IIRInput4: " << IIRInputMaster4 << " PREVIOUS IIRInput5: " << IIRInputMaster5 << endl;

    EV << "PREVIOUS IIROutput1: " << IIROutputMaster1 << " PREVIOUS IIROutput2: " << IIROutputMaster2 << " PREVIOUS IIROutput3: " << IIROutputMaster3 << " PREVIOUS IIROutput4: " << IIROutputMaster4 << " PREVIOUS IIROutput5: " << IIROutputMaster5 << endl;

    IIRInputMaster5 = IIRInputMaster4;
    IIRInputMaster4 = IIRInputMaster3;
    IIRInputMaster3 = IIRInputMaster2;
    IIRInputMaster2 = IIRInputMaster1;
    IIRInputMaster1 = IIRFilterInput;

    IIROutputMaster5 = IIROutputMaster4;
    IIROutputMaster4 = IIROutputMaster3;
    IIROutputMaster3 = IIROutputMaster2;
    IIROutputMaster2 = IIROutputMaster1;
    IIROutputMaster1 = (1/IIRden1) * ((IIRnum1 * IIRInputMaster1) + (IIRnum2 * IIRInputMaster2) + (IIRnum3 * IIRInputMaster3) + (IIRnum4 * IIRInputMaster4) + (IIRnum5 * IIRInputMaster5) - (IIRden2 * IIROutputMaster2) - (IIRden3 * IIROutputMaster3) - (IIRden4 * IIROutputMaster4) - (IIRden5 * IIROutputMaster5));

    IIRFilterOutputMaster = IIROutputMaster1;

    EV << "UPDATED IIRInput1: " << IIRInputMaster1 << " UPDATED IIRInput2: " << IIRInputMaster2 << " UPDATED IIRInput3: " << IIRInputMaster3 << " UPDATED IIRInput4: " << IIRInputMaster4 << " UPDATED IIRInput5: " << IIRInputMaster5 << endl;

    EV << "UPDATED IIROutput1: " << IIROutputMaster1 << " UPDATED IIROutput2: " << IIROutputMaster2 << " UPDATED IIROutput3: " << IIROutputMaster3 << " UPDATED IIROutput4: " << IIROutputMaster4 << " UPDATED IIROutput5: " << IIROutputMaster5 << endl;

    return IIRFilterOutputMaster;
}

double PCOClock::IIRFilterRelay(double IIRFilterInput)
{
    EV << "IIRFilterRelay..." << endl;
    EV << "PREVIOUS IIRInput1: " << IIRInputRelay1 << " PREVIOUS IIRInput2: " << IIRInputRelay2 << " PREVIOUS IIRInput3: " << IIRInputRelay3 << " PREVIOUS IIRInput4: " << IIRInputRelay4 << " PREVIOUS IIRInput5: " << IIRInputRelay5 << endl;

    EV << "PREVIOUS IIROutput1: " << IIROutputRelay1 << " PREVIOUS IIROutput2: " << IIROutputRelay2 << " PREVIOUS IIROutput3: " << IIROutputRelay3 << " PREVIOUS IIROutput4: " << IIROutputRelay4 << " PREVIOUS IIROutput5: " << IIROutputRelay5 << endl;

    IIRInputRelay5 = IIRInputRelay4;
    IIRInputRelay4 = IIRInputRelay3;
    IIRInputRelay3 = IIRInputRelay2;
    IIRInputRelay2 = IIRInputRelay1;
    IIRInputRelay1 = IIRFilterInput;

    IIROutputRelay5 = IIROutputRelay4;
    IIROutputRelay4 = IIROutputRelay3;
    IIROutputRelay3 = IIROutputRelay2;
    IIROutputRelay2 = IIROutputRelay1;
    IIROutputRelay1 = (1/IIRden1) * ((IIRnum1 * IIRInputRelay1) + (IIRnum2 * IIRInputRelay2) + (IIRnum3 * IIRInputRelay3) + (IIRnum4 * IIRInputRelay4) + (IIRnum5 * IIRInputRelay5) - (IIRden2 * IIROutputRelay2) - (IIRden3 * IIROutputRelay3) - (IIRden4 * IIROutputRelay4) - (IIRden5 * IIROutputRelay5));

    IIRFilterOutputRelay = IIROutputRelay1;

    EV << "UPDATED IIRInput1: " << IIRInputRelay1 << " UPDATED IIRInput2: " << IIRInputRelay2 << " UPDATED IIRInput3: " << IIRInputRelay3 << " UPDATED IIRInput4: " << IIRInputRelay4 << " UPDATED IIRInput5: " << IIRInputRelay5 << endl;

    EV << "UPDATED IIROutput1: " << IIROutputRelay1 << " UPDATED IIROutput2: " << IIROutputRelay2 << " UPDATED IIROutput3: " << IIROutputRelay3 << " UPDATED IIROutput4: " << IIROutputRelay4 << " UPDATED IIROutput5: " << IIROutputRelay5 << endl;

    return IIRFilterOutputRelay;
}


