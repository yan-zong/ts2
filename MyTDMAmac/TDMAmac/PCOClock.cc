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
    timestampVec.setName("Timestamp");

    // ---------------------------------------------------------------------------
    // Initialise variable
    // ---------------------------------------------------------------------------
    offset = par("offset"); // the clock offset
    drift =  par("drift");  // the clock skew (the variation of clock frequency)
    sigma1  = par("sigma1");    // the standard deviation of clock skew noise
    sigma2 = par("sigma2"); // the standard deviation of clock offset noise
    sigma3 = par("sigma3"); // the standard deviation of timestamp (meausmrenet) noise
    u3 = par("u3"); // the mean of timestamp (measurement) noise
    tau_0  = par("tau_0");  // clock update period
    Threshold = par("Threshold");
    pulseDuration = par("pulseDuration");
    ScheduleOffset = par("ScheduleOffset");
    tau = par("tau");   // the transmission delay
    alpha = par("alpha");   // the correction parameter of clock offset
    beta = par("beta");   // the correction parameter of clock skew
    CorrectionAlgorithm = par("CorrectionAlgorithm");    // correction algorithm
                                // 1 is for classic PCO by using constant value, 2 is for classic PCO by using offset value
    varepsilon = par("varepsilon");
    numRelay = par("numRelay");

    ClassicClock = 0;
    PCOClockState = 0;
    Timestamp = 0;   // timestamp based on the reception of SYNC packet
    LastUpdateTime = SIMTIME_DBL(simTime());
    ReceivedSYNCTime = 0;   // the reception time of SYNC packet
    offset_present = 0; // the present clock offset
    drift_present = 0;  // the present clock skew

    i = 0;

    NodeId = (findHost()->getId() - 4);
    EV << "PCOClock: the node id is " << NodeId << endl ;
    // id of master should be 0;
    // id of relay[0] should be 1; id of relay[1] should be 2;

    // EV << "yan: findHost()->getIndex() is " << findHost()->getIndex() << endl ;
    // EV << "yan: findHost()->getId() is " << findHost()->getId() << endl ;

    if(ev.isGUI())
    {
        updateDisplay();
    }

    if (NodeId <= numRelay)   // the sensor node is the master or relay nodes
    {
        scheduleAt(simTime() + ScheduleOffset + NodeId * pulseDuration ,new cMessage("CLTimer"));
        LastUpdateTime = ScheduleOffset + NodeId * pulseDuration;
        EV << "PCOClock: Clock starts at " << (simTime() + ScheduleOffset + NodeId * pulseDuration) << endl;
    }
    else    // the sensor node is the slave node, no need to implement the desynchronisation
    {
        scheduleAt(simTime(),new cMessage("CLTimer"));
        LastUpdateTime = 0;
        EV << "PCOClock: Clock starts at " << simTime() << endl;
    }

}

void PCOClock::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())
    {
        ClockUpdate();   // update PCO clock state
        EV << "PCOClock: PCO Clock State is " << PCOClockState << endl;

        i = i + 1;
        ev << "i = "<< i << endl;

        if(i % 100 == 0)
        {
            ev << "record the offset and drift of clock"<<endl;
            recordResult();
            driftStd.collect(drift);
            offsetStd.collect(offset);
        }

        scheduleAt(simTime()+ tau_0,new cMessage("CLTimer"));
    }

    else
    {
        error("Clock module receipts a packet from node. It does not receive packet from sensor node.\n");
    }

    delete msg;

    if(ev.isGUI())
    {
        updateDisplay();
    }
}

double PCOClock::ClockUpdate()
{
    double PCOClockStateTemp = 0;

    ev << "PCOClock: the PREVIOUS offset is "<< offset << endl;
    noise2 =  normal(0, sigma2, 1);
    offset = offset + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise2;
    offset_present = drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise2;
    ev << "PCOClock: the UPDATED offset is "<< offset << endl;
    ev << "PCOClock: the PRESENT offset is "<< offset_present << endl;

    ev << "PCOClock: the PREVIOUS drift is "<< drift <<endl;
    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    drift_present = noise1;
    ev << "PCOClock: the UPDATED drift is "<< drift <<endl;
    ev << "PCOClock: the PRESENT drift is "<< drift_present << endl;

    ev << "PCOClock: the PREVIOUS classic clock is "<< ClassicClock <<endl;
    // update the classic clock
    ClassicClock = ClassicClock + tau_0 + offset_present;
    ev << "PCOClock: the UPDATED classic clock is "<< ClassicClock <<endl;

    // update the PCO clock
    PCOClockStateTemp = PCOClockState + tau_0 + offset_present;
    ev << "PCOClock: the TEMP 'PCOClockState' is "<< PCOClockStateTemp <<endl;

    if ((PCOClockStateTemp) >= Threshold)
    {
        ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
        PCOClockState = PCOClockState + tau_0 + offset_present - Threshold;;
        ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;

        generateSYNC();
        EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

    }
    else if ((PCOClockStateTemp) < Threshold)
    {
        ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
        PCOClockState = PCOClockState + tau_0 + offset_present;
        ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
    }
    else
    {
        error("PCOClock: error in the PCO clock update function");
    }

    LastUpdateTime = SIMTIME_DBL(simTime());
    ev << "PCOClock: the 'LastUpdateTime' is "<< SIMTIME_DBL(simTime()) <<endl;

    pcoclockVec.record(PCOClockState);
    classicclockVec.record(ClassicClock);
    thresholdVec.record(Threshold);

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

double PCOClock::getTimestamp()
{
    ev << "PCOClock: PCOTimestamp... " << endl;
    noise3 = normal(u3, sigma3);
    noise3Vec.record(noise3);
    ev << "PCOClock: the timestamp noise 'noise3' is " << noise3 << endl;

    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    Timestamp = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + noise3;
    ev << "PCOClock: 'Timestamp' is " << Timestamp << endl;
    ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;

    timestampVec.record(Timestamp);

    return Timestamp;
}

void PCOClock::finish()
{
    recordScalar("sigma3",sigma3);
    recordScalar("drift_mean",driftStd.getMean());
    recordScalar("drift_std",driftStd.getStddev());
    recordScalar("offset_mean",offsetStd.getMean());
    recordScalar("offset_std",offsetStd.getStddev());
}

void PCOClock::updateDisplay()
{
    char buf[100];
    sprintf(buf, "offset [msec]: %3.2f   \ndrift [ppm]: %3.2f \norigine: %3.2f",
           offset,drift*1E6,LastUpdateTime);
    getDisplayString().setTagArg("t",0,buf);
}

void PCOClock::generateSYNC()
{
    Enter_Method_Silent(); // see simuutil.h for detail

    EV << "PCOClock: PCO clock state reaches threshold, it is reset to zero, meanwhile, a SYNC packet is generated \n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck -> setPtpType(SYNC);
    pck -> setByteLength(44);
    send(pck,"outclock");

    EV << "PCOClock: PCOClock transmits SYNC packet to Core module" << endl;
}

// Todo: double check this function, there are some bugs in this function.
double PCOClock::getMeasurementOffset(int MeasurmentAlgorithm, int AddressOffset)
{
    ev << "PCOClock: get measurement offset... "<< endl;

    double MeasuredOffset;

    // Note: 2.176E-3 (tau) means the time for receiver to recept the SYNC packet from the sender,
    // 1.82E-4 is for physical layer to check the SYNC packet
    // 2.176E-3 = 2.368E-3 - 1.82E-4

    // the 'getSource()' function of packet can be used to determine where is the received SYNC from

    // if the node is the relay node, the scheduled offset of DESYNC should be considered.
    if (MeasurmentAlgorithm == 1)   // the receipted SYNC is from master node
    {
        if (ReceivedSYNCTime < (Threshold/2))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - 0 + (ScheduleOffset + pulseDuration * NodeId);
        }
        else if ((ReceivedSYNCTime > (Threshold/2)) | (ReceivedSYNCTime == (Threshold/2)))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold + (ScheduleOffset + pulseDuration * NodeId);
        }
    }

    else if (MeasurmentAlgorithm == 2)  // node i receives the SYNC from node j (node i fires before node j)
    {
        if (ReceivedSYNCTime < (Threshold/2))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - 0 - (pulseDuration * AddressOffset);
        }
        else if ((ReceivedSYNCTime > (Threshold/2)) | (ReceivedSYNCTime == (Threshold/2)))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold - (pulseDuration * AddressOffset);
        }
    }

    else if (MeasurmentAlgorithm == 3)  // node j receives the SYNC from node i (node i fires before node j)
    {
        if (ReceivedSYNCTime < (Threshold/2))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - 0 + (pulseDuration * AddressOffset);
        }
        else if ((ReceivedSYNCTime > (Threshold/2)) | (ReceivedSYNCTime == (Threshold/2)))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold + (pulseDuration * AddressOffset);
        }
    }

    else if (MeasurmentAlgorithm == 4)  // the node is the slave node, the scheduled offset of DESYNC should not be considered.
    {
        if (ReceivedSYNCTime < (Threshold/2))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - 0;
        }
        else if ((ReceivedSYNCTime > (Threshold/2)) | (ReceivedSYNCTime == (Threshold/2)))
        {
            MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold;
        }

    }

    else
    {
        error("error in the offset measurement.\n");
    }

    ev << "PCOClock: 'MeasurementOffset' is " << MeasuredOffset << endl;

    measurementoffsetVec.record(MeasuredOffset);

    return MeasuredOffset;
}

double PCOClock::getMeasurementSkew(double measuredOffset)
{
    double MeasuredSkew;

    MeasuredSkew = measuredOffset / Threshold;

    return MeasuredSkew;
}

double PCOClock::setReceivedSYNCTime(double value)
{
    ReceivedSYNCTime = value;
    return ReceivedSYNCTime;
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

void PCOClock::adjustClock(double estimatedOffset, double estimatedSkew)
{
    double PCOClockStateTemp = 0;

    if (CorrectionAlgorithm == 0)   // null
    {

    }

    else if (CorrectionAlgorithm == 1)   // correct PCO state by using constant value, i.e., classic PCO
    {
        PCOClockStateTemp = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon;

        ev << "PCOClock: the TEMP 'PCOClockState' is "<< PCOClockStateTemp <<endl;

        if ((PCOClockStateTemp) >= Threshold)
        {
            ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
            PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon - Threshold ;
            ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
            ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;

            generateSYNC();

            EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

        }
        else if ((PCOClockStateTemp) < Threshold)
        {
            ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
            PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon;
            ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
            ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;
        }
        else
        {
            error("PCOClock: error in the PCO clock adjustment function");
        }

        ev << "PCOClock: the PCO clock state is adjusted to " << PCOClockState << endl;
    }

    else if (CorrectionAlgorithm == 2)  // correct PCO state by using clock offset and skew, i.e., PkCOs
    {
        // correct the PCO clock state
        PCOClockState = PCOClockState + alpha * estimatedOffset;

        ev << "PCOClock: the PCO clock state is adjusted to " << PCOClockState << endl;

    }
    else if (CorrectionAlgorithm == 3)  // correct PCO state by using clock offset and skew by using the P controller, i.e., PkCOs
    {
        // correct the PCO threshold
        Threshold = Threshold + alpha * estimatedOffset;
        // correct the PCO skew
        drift = drift + beta * estimatedSkew;

        ev << "PCOClock: the PCO clock threshold is adjusted to " << Threshold << endl;
        ev << "PCOClock: the PCO clock skew is adjusted to " << drift << endl;

    }
    else if (CorrectionAlgorithm == 4)  // new clock correction algorithm
    {

    }

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


