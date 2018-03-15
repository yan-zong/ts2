//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  07 Nov 2016
// * Author:      Yan Zong, Xuewu Dai
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
#include "Packet_m.h"

Define_Module(PCOClock);

void PCOClock::initialize()
{
    // ---------------------------------------------------------------------------
    // Initialize variable for saving output data
    // ---------------------------------------------------------------------------
    driftVec.setName("drift");
    offsetVec.setName("offset");
    noise1Vec.setName("noise1");
    noise2Vec.setName("noise2");
    noise3Vec.setName("noise3");
    pcoclockVec.setName("PCOclockstate");
    classicclockVec.setName("classicclock");
    thresholdVec.setName("threshold");
    update_numberVec.setName("update_number");
    measuredoffsetmasterVec.setName("measuredoffsetMaster");
    measuredoffsetrelayVec.setName("measuredoffsetRelay");
    timestampVec.setName("timestamp");
    PCOfireTimeVec.setName("PCOfiretime");

    // ---------------------------------------------------------------------------
    // Initialize variable
    // ---------------------------------------------------------------------------
    offset = par("offset"); // the clock offset
    drift =  par("drift");  // the clock skew (the variation of clock frequency)
    sigma1  = par("sigma1");    // the standard deviation of clock skew noise
    sigma2 = par("sigma2"); // the standard deviation of clock offset noise
    sigma3 = par("sigma3"); // the standard deviation of timestamp (meausmrenet) noise
    u3 = par("u3"); // the mean of timestamp (measurement) noise
    tau_0  = par("tau_0");  // clock update period
    Threshold = par("Threshold");   // PCO clock state threshold
    pulseDuration = par("pulseDuration");   // duration of SYNC packet from neighboring relay nodes
    ScheduleOffset = par("ScheduleOffset"); // duration of ScheduledOffset from proposed superframe
    tau = par("tau");   // the transmission delay
    alpha = par("alpha");   // the correction parameter of clock offset
    beta = par("beta");   // the correction parameter of clock skew
    CorrectionAlgorithm = par("CorrectionAlgorithm");    // correction algorithm
                                // 1 is for classic PCO by using constant value, 2 is for classic PCO by using offset value
    varepsilon = par("varepsilon"); // the coupling strength of PCO model
    refractory = par("refractory"); // refractory period

    ClassicClock = 0;
    PCOClockState = offset;
    Timestamp = 0;   // timestamp based on the reception of SYNC packet
    LastUpdateTime = 0;
    ReceivedSYNCTime = 0;   // the reception time of SYNC packet
    offset_present = 0; // the present clock offset
    drift_present = 0;  // the present clock skew
    LastFireTime = 0;

    i = 0;

    offsetOutPContr = 0; // for P controller
    skewOutPContr = 0;   // for P controller

    offsetOutIContr = 0; // for I controller
    skewOutIContr = 0;   // for I controller

    offsetOutPIContr = 0;    // for PI controller
    skewOutPIContr = 0;  // for PI controller

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

    cModule* RelayNode = NULL;
    numRelay = 0;
    /*
    do{
        numRelay++;
        RelayNode = findHost() -> getParentModule() -> getSubmodule("rnode", numRelay);
    }while(RelayNode);
    */
    while(RelayNode)
    {
        numRelay++;
        RelayNode = findHost() -> getParentModule() -> getSubmodule("rnode", numRelay);
    }
    EV << "PCOClock: the number of relay node is " << numRelay << endl;

    // the desynchronisation technology should be implemented into the master and relay nodes,
    // the desynchronisation cannot be implemented into the slave nodes, because slave nodes cannot
    // transmit the packet, they just can receipt the packet from other nodes.

    if (NodeId <= numRelay)   // the sensor node is the master or relay node
    {
        scheduleAt(simTime() + ScheduleOffset + NodeId * pulseDuration ,new cMessage("CLTimer"));
        LastUpdateTime = ScheduleOffset + NodeId * pulseDuration;
        EV << "PCOClock: Clock starts at " << (simTime() + ScheduleOffset + NodeId * pulseDuration) << endl;
        EV << "PCOClock: the LastUpdateTime is " << LastUpdateTime <<endl;
    }
    else    // the sensor node is the slave node
    {
        // implementation of desynchronisation
        scheduleAt(simTime() + ScheduleOffset + (NodeId - numRelay) * pulseDuration ,new cMessage("CLTimer"));
        LastUpdateTime = ScheduleOffset + (NodeId - numRelay) * pulseDuration;
        EV << "PCOClock: Clock starts at " << (simTime() + ScheduleOffset + (NodeId - numRelay) * pulseDuration) << endl;
        EV << "PCOClock: the LastUpdateTime is " << LastUpdateTime <<endl;

        // no implementation of desynchronisation
        // scheduleAt(simTime(),new cMessage("CLTimer"));
        // LastUpdateTime = 0;
        // EV << "PCOClock: Slave clock starts at " << simTime() << ", and LastUpdateTime is "<< LastUpdateTime << endl;
    }

}

void PCOClock::handleMessage(cMessage *msg)
{
    if(msg -> isSelfMessage())
    {
        ClockUpdate();   // key function -- update PCO clock state
        EV << "PCOClock: PCO Clock State is " << PCOClockState << endl;

        i = i + 1;
        ev << "PCOClock: i = "<< i << endl;

        if(i % 100 == 0)
        {
            ev << "PCOClock: record the offset and drift of clock"<<endl;
            recordResult();
            driftStd.collect(drift);
            offsetStd.collect(offset);
        }

        scheduleAt(simTime()+ tau_0,new cMessage("CLTimer"));
    }

    else
    {
        error("Clock module receipts a packet from other modules. It does not receive packet from sensor node.\n");
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
    ev << "PCOClock: the UPDATED offset is "<< offset << ", and noise2 is " << noise2 <<endl;
    ev << "PCOClock: the PRESENT offset is "<< offset_present << ", and (SIMTIME_DBL(simTime()) - LastUpdateTime) should be tau_0, and it actually is "<< (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;

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
        PCOClockState = 0;
        // PCOClockState = PCOClockState + tau_0 + offset_present - Threshold;
        ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;

        generateSYNC();

        LastFireTime = SIMTIME_DBL(simTime());
        PCOfireTimeVec.record(LastFireTime);

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
    ev << "PCOClock: Timestamp... " << endl;
    noise3 = normal(u3, sigma3);
    noise3Vec.record(noise3);
    ev << "PCOClock: the timestamp noise 'noise3' is " << noise3 << endl;

    ev << "PCOClock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;
    ev << "PCOClock: (SIMTIME_DBL(simTime()) - LastUpdateTime) = "<< (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;;

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

    EV << "PCOClock: PCO clock state reaches threshold, it is reset to zero, meanwhile, a SYNC packet is generated. \n";

    Packet *pck = new Packet("SYNC");
    pck -> setPacketType(SYNC);
    pck -> setByteLength(TIMESTAMP_BYTE);
    pck -> setData(Threshold);  // for PI controller
    send(pck, "outclock");

    EV << "PCOClock: the threshold transmitted by packet is " << pck -> getData() <<endl;
    EV << "PCOClock: PCOClock transmits SYNC packet to Core module" << endl;
}

double PCOClock::getMeasurementOffset(int MeasurmentAlgorithm, int AddressOffset)
{
    ev << "PCOClock: get measurement offset... "<< endl;

    double MeasuredOffset;

    double NormalisedSYNCTime = 0;    // Normalise ReceivedSYNCTime

    if (MeasurmentAlgorithm == 1)   // relay node receive the SYNC from master node.
    {

        NormalisedSYNCTime = ReceivedSYNCTime / Threshold;

        MeasuredOffset = (NormalisedSYNCTime - tau) - 0 + (ScheduleOffset + pulseDuration * NodeId);

        if (MeasuredOffset > 0.5)
            MeasuredOffset = (NormalisedSYNCTime - tau) - 1 + (ScheduleOffset + pulseDuration * NodeId);

        // if (MeasuredOffset > (Threshold/2))
        //        MeasuredOffset = (NormalisedSYNCTime - tau) - (Threshold / Threshold) + (ScheduleOffset + pulseDuration * NodeId);
    }

    else if (MeasurmentAlgorithm == 2)  // relay node i receives the SYNC from relay node j (node i fires before node j)
    {
        NormalisedSYNCTime = ReceivedSYNCTime / Threshold;

        MeasuredOffset = (NormalisedSYNCTime - tau) - 0 - (pulseDuration * AddressOffset);

        if (MeasuredOffset > 0.5)
            MeasuredOffset = (NormalisedSYNCTime - tau) - 1 - (pulseDuration * AddressOffset);

        // MeasuredOffset = (ReceivedSYNCTime - tau) - 0 - (pulseDuration * AddressOffset);

        // if (MeasuredOffset > (Threshold/2))
        //     MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold - (pulseDuration * AddressOffset);

    }

    else if (MeasurmentAlgorithm == 3)  // relay node j receives the SYNC from relay node i (node i fires before node j)
    {
        NormalisedSYNCTime = ReceivedSYNCTime / Threshold;

        MeasuredOffset = (NormalisedSYNCTime - tau) - 0 + (pulseDuration * AddressOffset);

        if (MeasuredOffset > 0.5)
            MeasuredOffset = (NormalisedSYNCTime - tau) - 1 + (pulseDuration * AddressOffset);

        // MeasuredOffset = (ReceivedSYNCTime - tau) - 0 + (pulseDuration * AddressOffset);

        // if (MeasuredOffset > (Threshold/2))
        //    MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold + (pulseDuration * AddressOffset);

    }

    else if (MeasurmentAlgorithm == 4)  // slave node receive the SYNC from relay node.
    {
        // todo: this 'else-if' loop needs to be updated to fix the bug.
        NormalisedSYNCTime = ReceivedSYNCTime / Threshold;

        MeasuredOffset = (NormalisedSYNCTime - tau) - 0 + (pulseDuration * AddressOffset);

        if (MeasuredOffset > 0.5)
            MeasuredOffset = (NormalisedSYNCTime - tau) - 1 + (pulseDuration * AddressOffset);


        // MeasuredOffset = (ReceivedSYNCTime - tau) - 0 - (ScheduleOffset + pulseDuration * AddressOffset);

        // if (MeasuredOffset > (Threshold/2))
        //    MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold - (ScheduleOffset + pulseDuration * AddressOffset);

    }

    else if (MeasurmentAlgorithm == 5)  // slave receive SYNC from master node.
    {
        NormalisedSYNCTime = ReceivedSYNCTime / Threshold;

        MeasuredOffset = (NormalisedSYNCTime - tau) - 0 + (ScheduleOffset + pulseDuration * (NodeId - numRelay));

        if (MeasuredOffset > 0.5)
            MeasuredOffset = (NormalisedSYNCTime - tau) - 1 + (ScheduleOffset + pulseDuration * (NodeId - numRelay));

        // MeasuredOffset = (ReceivedSYNCTime - tau) - 0;

        // if (MeasuredOffset > (Threshold/2))
        //  MeasuredOffset = (ReceivedSYNCTime - tau) - Threshold;

    }

    else
    {
        error("error in the offset measurement.\n");
    }

    ev << "PCOClock: 'MeasurementOffset' is " << MeasuredOffset << endl;

    if (MeasurmentAlgorithm == 1 || MeasurmentAlgorithm == 5)
        measuredoffsetmasterVec.record(MeasuredOffset);
    else if (MeasurmentAlgorithm == 2 || MeasurmentAlgorithm == 3 || MeasurmentAlgorithm == 4)
        measuredoffsetrelayVec.record(MeasuredOffset);

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
        if ((SIMTIME_DBL(simTime()) - LastFireTime) <= refractory)
        {
            ev << "PCOClock: during the refractory, no adjustment in the PCO clock state. "<< endl;
            return;

        }
        else if ((SIMTIME_DBL(simTime()) - LastFireTime) > refractory)
        {
            ev << "PCOClock: out the refractory, no adjustment in the PCO clock state. "<< endl;

            PCOClockStateTemp = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon;

            ev << "PCOClock: the TEMP 'PCOClockState' is "<< PCOClockStateTemp <<endl;

            if ((PCOClockStateTemp) >= Threshold)
            {
                ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
                PCOClockState = 0;
                // PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon - Threshold ;
                ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState << ", due to the varepsilon is " << varepsilon <<endl;
                ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;

                generateSYNC();

                LastFireTime = SIMTIME_DBL(simTime());
                PCOfireTimeVec.record(LastFireTime);

                EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

            }
            else if ((PCOClockStateTemp) < Threshold)
            {
                ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
                PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + varepsilon;
                ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState << ", due to the varepsilon is " << varepsilon <<endl;
                ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;
            }
            else
            {
                error("PCOClock: error in the PCO clock adjustment function");
            }

            ev << "PCOClock: the PCO clock state is adjusted to " << PCOClockState << endl;
        }
    }

    else if (CorrectionAlgorithm == 2)  // correct PCO state by using measurement offset
    {

        // correct the PCO clock state
        PCOClockStateTemp = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + alpha * estimatedOffset;

        ev << "PCOClock: the TEMP 'PCOClockState' is "<< PCOClockStateTemp <<endl;

        if ((PCOClockStateTemp) >= Threshold)
        {
            ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
            PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + alpha * estimatedOffset - Threshold ;
            ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
            ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;

            generateSYNC();

            LastFireTime = SIMTIME_DBL(simTime());
            PCOfireTimeVec.record(LastFireTime);

            EV << "PCOClock: generate and sent SYNC packet to Core module. " << endl;

        }
        else if ((PCOClockStateTemp) < Threshold)
        {
            ev << "PCOClock: the PREVIOUS 'PCOClockState' is "<< PCOClockState <<endl;
            PCOClockState = PCOClockState + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) + alpha * estimatedOffset;
            ev << "PCOClock: the UPDATED 'PCOClockState' is "<< PCOClockState <<endl;
            ev << "PCOClock: the presented updated PCO clock state 'drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)' is " << drift * (SIMTIME_DBL(simTime()) - LastUpdateTime) << endl;
        }
        else
        {
            error("PCOClock: error in the PCO clock adjustment function");
        }

        ev << "PCOClock: the PCO clock state is adjusted to " << PCOClockState << endl;

    }

    else if (CorrectionAlgorithm == 3)  // correct PCO state threshold and skew by using the PkCOs with P controller
    {
        // correct the PCO threshold
        // Threshold = Threshold + alpha * estimatedOffset;
        Threshold = Threshold + alpha * estimatedOffset * Threshold;    // denormalize estimated the offset and adjust the threshold
        // correct the PCO skew
        // drift = drift + beta * estimatedSkew;
        drift = drift + beta * estimatedSkew * Threshold;   // denormalize estimated the skew and adjust the skew

        ev << "PCOClock: the PCO clock threshold is adjusted to " << Threshold << endl;
        ev << "PCOClock: the PCO clock skew is adjusted to " << drift << endl;

    }

    else if (CorrectionAlgorithm == 4)  // correct PCO state threshold and skew by using the PkCOs with PI controller
    {
        // for P controller
        // offsetOutPContr = alpha * estimatedOffset;
        // skewOutPContr = beta * estimatedSkew;
        offsetOutPContr = alpha * estimatedOffset * Threshold;  // denormalize the estimated offset
        skewOutPContr = beta * estimatedSkew * Threshold;   // denormalize the estimated skew

        ev << "PCOClock: the output of P controller for offset is " << offsetOutPContr << endl;
        ev << "PCOClock: the output of P controller for skew is " << skewOutPContr << endl;

        // for I controller
        ev << "PCOClock: the output of I controller for offset WAS " << offsetOutIContr << endl;
        ev << "PCOClock: the output of I controller for skew WAS " << skewOutIContr << endl;

        // offsetOutIContr = offsetOutIContr + alpha * estimatedOffset;
        // skewOutIContr = skewOutIContr + beta * estimatedSkew;

        offsetOutIContr = offsetOutIContr + alpha * estimatedOffset * Threshold;    // denormalize the estimated offset
        skewOutIContr = skewOutIContr + beta * estimatedSkew * Threshold;   // denormalize the estimated skew

        ev << "PCOClock: the output of I controller for offset IS " << offsetOutIContr << endl;
        ev << "PCOClock: the output of I controller for skew IS " << skewOutIContr << endl;

        // sum the P controller and I controller
        offsetOutPIContr = offsetOutPContr + offsetOutIContr;
        skewOutPIContr = skewOutPContr + skewOutIContr;

        Threshold = Threshold + offsetOutPIContr;   // correct the PCO threshold
        drift = drift + skewOutPIContr; // correct the PCO skew

        ev << "PCOClock: the PCO clock threshold is adjusted to " << Threshold << endl;
        ev << "PCOClock: the PCO clock skew is adjusted to " << drift << endl;
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


