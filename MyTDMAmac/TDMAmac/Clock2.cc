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

#include "Clock2.h"
#include "Constant.h"
#include "PtpPkt_m.h"

Define_Module(Clock2);

void Clock2::initialize()
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
    adjustedthresholdvalueVec.setName("ThresholdAdjustValue");    // the adjusted value of threshold
    thresholdVec.setName("RegisterThreshold");    // the threshold value
    thresholdOffsetVec.setName("ThresholdOffset");
    pulsetimeVec.setName("PulseTime");



    // ---------------------------------------------------------------------------
    // Initialise variable
    // ---------------------------------------------------------------------------
    offset = par("offset");
    drift =  par("drift");
    sim_time_limit = par("sim_time_limit");
    error_drift = offset;
    error_offset = drift;
    sigma1  = par("sigma1");
    sigma2 = par("sigma2");
    sigma3 = par("sigma3");
    u3 = par("u3");
    Tcamp  = par("Tcamp");
    Tsync = par("Tsync");
    alpha = par("alpha");
    beta = par("beta");
    RegisterThreshold = par("RegisterThreshold");
    FrameDuration = par ("FrameDuration");
    slotDuration = par("slotDuration");
    ScheduleOffset = par("ScheduleOffset");
    delay = par("delay");
    EV << "Clock: delay is " << delay << endl;

    AdjustParameter = par("AdjustParameter");
    EV << "Clock: AdjustParameter is " << AdjustParameter << endl;


    // ---------------------------------------------------------------------------
    // Initialise variable
    // ---------------------------------------------------------------------------
    lastupdatetime = SIMTIME_DBL(simTime());
    phyclock = softclock = offset;
    drift_previous = drift;
    offset_previous = offset;
    i = 0;
    j = 0;
    delta_drift = delta_offset = 0;
    k = int(sim_time_limit/Tcamp);
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
    numFires = 1;
    numPulse = 0;
    LastUpdateTime = SIMTIME_DBL(simTime());
    offsetStore = 0;    // offset is set to zero when PCO time is greater than threshold, store the offset
    ThresholdAdjustValue = 0;
    RefTimePreviousPulse = 0;
    offsetTotal = 0;
    ReceivedPulseTime = 0;

    NodeId = (findHost()->getIndex() + 1);
    EV << "Clock: the node id is " << NodeId << ", and ScheduleOffset*NodeId is "<< ScheduleOffset*NodeId <<endl ;
    // id of relay[0] should be 1; id of relay[1] should be 2;

    ClockOffset = 0;
    ThresholdOffset = 0;

    // ---------------------------------------------------------------------------
    // Initialise variable for Kalman Filter
    // ---------------------------------------------------------------------------
    //(1)A[2][2]={{1,Tsync},{0,1}}
     A[0][0] = 1;
     A[0][1] = Tsync;
     A[1][0] = 0;
     A[1][1] = 1;
     //(2)B[2][2]={{-1,-Tsync},{0,-1}}
     B[0][0] = -1;
     B[0][1] = -Tsync;
     B[1][0] = 0;
     B[1][1] = -1;
    //(3)H[2][2]={{1,0},{0,1}}
     H[0][0]=1;
     H[0][1]=0;
     H[1][0]=0;
     H[1][1]=1;
    //(4)Q[2][2]={{s2*s2,0},{0,s1*s1}}
     Q[0][0] = sigma2*sigma2;
     Q[0][1] = 0;
     Q[1][0] = 0;
     Q[1][1] = sigma1*sigma1;
    //(5)R[2][2]={{0.5*s3*s3,0.5*s3*s3/Tsync},{0.5*s3*s3/Tsync,2*(0.5*s3*s3/(Tsync*Tsync))}}
    /* R[0][0] = 0.5*sigma3*sigma3;
     R[0][1] = (0.5*sigma3*sigma3-u3*u3)/Tsync;
     R[1][0] = (0.5*sigma3*sigma3-u3*u3)/Tsync;
     R[1][1] = 2*(0.5*sigma3*sigma3/(Tsync*Tsync));*/
     R[0][0] = 0.5*sigma3*sigma3;
     R[0][1] = 0.5*sigma3*sigma3/Tsync;
     R[1][0] = 0.5*sigma3*sigma3/Tsync;
     R[1][1] = 2*(0.5*sigma3*sigma3/(Tsync*Tsync));
   //(6)xkhat[2][1]={{Tsync*drift},{drift}}
     xkhat[0][0] = 0; // Tsync*drift;
     xkhat[1][0] = drift;
   //(7)ukhat[2][1]={0,0}
     ukhat[0][0] = 0;
     ukhat[1][0] = 0;
   //(8)Pk[2][2]=Q[2][2]
   for(int m=0;m<=1;m++)
   {
       for(int n=0;n<=1;n++)
       {
           Pk[m][n] = Q[m][n];
       }
   }

   // openfile();
   // ---------------------------------------------------------------------------
   // Initialise timer, the sampling interval is 'Tcamp' (See omnetpp.ini)
   // the real time clock frequency is 32.768 KHz, and the corresponding
   // period is 1/(32.768k) = 30.51757813 us. Therefore, the clock update period is
   // 30.51757813 us, (configure in omnetpp.ini)
   // ---------------------------------------------------------------------------
   if(ev.isGUI())
   {
       updateDisplay();
   }

   //TODO::??131
   //recordResult();

   delta_driftVec.record(delta_drift);
   delta_offsetVec.record(delta_offset);

   // Tcamp is clock update period
   // scheduleAt(simTime() + NodeId*slotDuration + ScheduleOffset,new cMessage("CLTimer"));
   // scheduleAt(simTime() + ScheduleOffset,new cMessage("CLTimer"));
   scheduleAt(simTime() + ScheduleOffset + NodeId*slotDuration ,new cMessage("CLTimer"));
   // LastUpdateTime = NodeId*ScheduleOffset;
   LastUpdateTime = ScheduleOffset + NodeId*slotDuration;
   RefTimePreviousPulse = LastUpdateTime;
   EV << "Clock: Clock starts at " << (simTime() + NodeId*slotDuration + ScheduleOffset) << endl;
}

void Clock2::handleMessage(cMessage *msg)
{
    if(msg -> isSelfMessage())
    {
        // ---------------------------------------------------------------------------
        // Timer. Viene salvato il valore del clock in uscita, ottenuto mediante la
        // funzione interna getTimestamp(). Viene ripristinato il timer a 1.0 s.
        // ---------------------------------------------------------------------------
        //TODO:??
        Phyclockupdate();   // update physical clock by clock offset and drift
        EV << "Clock: phyclock = " << phyclock << endl;

        i = i + 1;
        ev << "i = "<< i << endl;
        ev << "k = " << k << endl;
        /*¼ÓifÅÐ¶ÏÓï¾ä£¬ÔÚsim_time_limit/Tcamp½ÏÐ¡Ês±¿ÉÒÔ¼ÇÂ¼½Ï¶àÊý¾Ý*/

        if(i % 10 == 0)
        {
            ev << "count delta_drfit and delta_offset:" << endl;
            delta_drift = drift - drift_previous;
            delta_offset = offset - offset_previous;
            if(k >= 9999999)
            {
                ev<<"compare 1 success!"<<endl;
                if(i%100==0)
                {
                    ev<<"Larger amount of data,record delta_offset and delta_drift."<<endl;
                    delta_driftVec.record(delta_drift);
                    delta_offsetVec.record(delta_offset);
                }
            }
            else
            {
                ev<<"less amount of data£¬record delta_offset and delta_drift."<<endl;
                delta_driftVec.record(delta_drift);
                delta_offsetVec.record(delta_offset);
            }

            drift_previous = drift;
            offset_previous = offset;
        }

        if(k >= 9999999)
        {
            ev<<"compare 2 success!"<<endl;
            if((i>10)&&(i%100==0))
            {
                ev<<"Larger amount of data,record offset and drift of updatePhyclock"<<endl;
                recordResult();
                driftStd.collect(drift);
                offsetStd.collect(offset);
            }
        }
        else
        {
            ev<<"Less amount of data,record offset and drift of updatePhyclock"<<endl;
            recordResult();
            //TODO:Í³¼ÆdriftºÍoffsetµÄÖµ
            driftStd.collect(drift);
            offsetStd.collect(offset);
        }

        scheduleAt(simTime()+ Tcamp,new cMessage("CLTimer"));
    }

    else
    {
        error("Clock2 gets a Packet from node. It now does not exchange Packet with node.\n");
    // ---------------------------------------------------------------------------
    // Messaggio ricevuto dal sistema (nodo a cui il clock si riferisce).
    // Il messaggio ricevuto puo` essere:
    // - un messaggio di tipo TIME_REQ, con il quale il sistema interroga il clock
    //   per ottenere una misura di tempo, vale a dire un timestamp;
    // - un messaggio di tipo OFFSET_ADJ, con il quale viene fatta una correzione
    //   dell'offset del clock;
    // - un messaggio di tipo FREQ_ADJ, con il quale viene fatta una correzione del
    //   tick rate del clock.
    // ---------------------------------------------------------------------------
        ev<<"A Packet received by clock to gettimestamp.\n";
        switch(((Packet *)msg)->getClockType()){
            case TIME_REQ:
                ev<<"TIME_REQ Packet received by clock.\n";
                msg->setName("TIME_RES");
                //TODO:
                switch(((Packet *)msg)->getPtpType()){
                case SYNC:
                    ev<<"T2 gettimestamp"<<endl;
                Tm= ((Packet *)msg)->getData();
                ev<<"clock_Tm= "<<Tm<<endl;
                    softclock_t2 = getTimestamp();
                    ev<<"softclock_t2= "<<softclock_t2<<endl;
                    msg->setTimestamp(softclock_t2);
                    softclock_t2Vec.record(softclock_t2);
                    break;
                case DREQ:
                    ev<<"T3 gettimestamp"<<endl;
                    softclock_t3 = getTimestamp();
                    ev<<"softclock_t3= "<<softclock_t3<<endl;
                    msg->setTimestamp(softclock_t3);
                    softclock_t3Vec.record(softclock_t3);
                    break;
                }

                ((Packet *)msg)->setClockType(TIME_RES);
                send((cMessage *)msg->dup(),"outclock");//
                break;
            case OFFSET_ADJ:    //correzione dell'offset
                adjtimex(((Packet *)msg)->getData(),0);
                break;
            case FREQ_ADJ:      //correzione del drift
                //ev<<"FREQ_ADJ Packet received by clock.\n";
                adjtimex(((Packet *)msg)->getData(),1);
                break;
        }
        if((((Packet *)msg)->getClockType()==FREQ_ADJ)){
             ev<<"adjust offset and drift:"<<endl;
             adj_offset_drift(); // dxw->hyw: see line 227 sdjtimex(, 1),
                                 // we process the FREQ_ADJ packet two times??
        }
    }

    delete msg;

    if(ev.isGUI())
    {
        updateDisplay();
    }
}

/* when the clock time reach the threshold value, the clock time will be reset to zero */
double Clock2::Phyclockupdate()
{
    ev << "Clock: update clock, the PREVIOUS offset is "<< offset << ", and PREVIOUS drift is "<< drift <<endl;

    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    ev << "Clock: the UPDATED drift is "<< drift <<endl;

    noise2 =  normal(0,sigma2,1);
    offset = offset + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime)+ noise2;
    ev << "Clock: the UPDATED offset is "<< offset << endl;

    ev << "Clock: the PREVIOUS physical clock time is " << phyclock << endl;
    ev << "Clock: RegisterThreshold - Tcamp = " << (RegisterThreshold - Tcamp) << endl;
    ev << "Clock: phyclock - RegisterThreshold = " << (phyclock - RegisterThreshold) << endl;
    ev << "(phyclock - (RegisterThreshold - Tcamp)) = " << (phyclock - (RegisterThreshold - Tcamp)) << endl;

    // if (phyclock - (RegisterThreshold - Tcamp) > (-1E-6))
    if (((phyclock - RegisterThreshold) > (-30.51757813E-6)) | ((phyclock - RegisterThreshold) == (-30.51757813E-6)))
    {
        numPulse = numPulse + 1;
        RefTimePreviousPulse = SIMTIME_DBL(simTime());
        offsetTotal = offsetTotal + offset; // record the offset
        EV << "Clock: the 'numPulse' is "<< numPulse <<endl;
        EV << "Clock: the 'RefTimePreviousPulse' is "<< RefTimePreviousPulse <<endl;
        EV << "Clock: the 'offsetTotal' is "<< offsetTotal <<endl;

        PulseTime = SIMTIME_DBL(simTime());
        pulsetimeVec.record(PulseTime);

        phyclock = 0;
        offset = 0;
        EV << "Clock: because the clock time is greater than threshold value " << RegisterThreshold;
        EV << ", the clock time is RESET to " << phyclock;
        EV << ", and the offset is also RESET to " << offset << endl;

        generateSYNC();
        EV << "Clock: generate and sent SYNC packet to Core module. " << endl;

    }
    else
    {
        phyclock = offset + (SIMTIME_DBL(simTime()) - RefTimePreviousPulse);
        ev << "Clock: based on the UPDATED offset: "<< offset << ", the UPDATED physical clock time is " << phyclock << endl;
    }

    LastUpdateTime = SIMTIME_DBL(simTime());
    ev << "Clock: the variable 'LastUpdateTime' is "<< SIMTIME_DBL(simTime()) <<endl;
    return phyclock;
}
void Clock2::recordResult(){
    driftVec.record(drift);
    offsetVec.record(offset);
    update_numberVec.record(i);
    noise1Vec.record(noise1);
    noise2Vec.record(noise2);

    adjustedthresholdvalueVec.record(ThresholdAdjustValue);    // the adjusted value of threshold
    thresholdVec.record(RegisterThreshold);    // the threshold value
    phyclockVec.record(phyclock);
    thresholdOffsetVec.record(ThresholdOffset);



}

/* @breif get timestamp of PCO */
double Clock2::getPCOTimestamp()
{
    ev << "Clock: PCO Timestamp " << endl;
    ev << "Clock: simTime = " << SIMTIME_DBL(simTime()) << ", LastUpdateTime = "<< LastUpdateTime << endl;

    double PCOClock = phyclock + drift * (SIMTIME_DBL(simTime()) - LastUpdateTime);
    ev << "PCOClock = " << PCOClock << endl;

    softclock = PCOClock;
    ev << "softclock = " << softclock << endl;

    softclockVec.record(softclock);
    ev << "Clock: the returned PCOClock time is " << softclock <<endl;
    return softclock;
}

/* @breif get timestamp of local drifting clock */
double Clock2::getTimestamp()
{
    noise3 = normal(u3,sigma3);
    noise3Vec.record(noise3);
    // double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal + noise3;
    double clock = getPCOTimestamp() + numPulse * FrameDuration + offsetTotal;

    ev << "Clock: numPulse * FrameDuration = " << numPulse * FrameDuration;
    ev << ", the variable 'offsetTotal' is " << offsetTotal <<endl;
    ev << ", the returned clock time is " << clock <<endl;
    return clock;
}



// for PTP
/*
double Clock2::getTimestamp()
{
    //TODO:Modify
    ev << "get Timestamp " << endl;
    ev << "simTime = " << SIMTIME_DBL(simTime()) << " lastupdatetime = "<< lastupdatetime << endl;
    double clock = offset + drift * (SIMTIME_DBL(simTime()) - lastupdatetime) + SIMTIME_DBL(simTime());
    ev << "clock = " << clock << endl;
    noise3 = normal(u3,sigma3);
    ev << " noise3 = "<< noise3 <<endl;
    noise3Vec.record(noise3);
    softclock = clock ;//+ noise3;
    ev << "softclock = " << softclock << endl;
    softclockVec.record(softclock);
    //ev.printf("phyclock_float=%8f",phyclock_float);
    return softclock;
}
*/


void Clock2::adjtimex(double value, int type)
{
    switch(type)
    {
        case 0: // adjust the local clock offset
        {
            /*noise3 = normal(0,sigma3);
            ev<<"noise3= "<<noise3<<endl;
            noise3Vec.record(noise3);
            offset_adj_value = value + noise3;*/
            offset_adj_value = value;
            ev << "offset_value = " << value << endl;
            ev << "offset_adj_value = "<< offset_adj_value << endl;
            break;
        }
        case 1: // adjust the local clock drift
        {
            ev << "clock_Tm_previous = " << Tm_previous << endl;
            ev << "clock_Tm - clock_Tm_previous = "<< Tm - Tm_previous << endl;
            drift_adj_value = value + offset_adj_previous/(Tm - Tm_previous);
            //drift_adj_value = offset_adj_value/Tsync;
            ev << " drift_value = " << value << endl;
            ev << " drift_adj_value = " << drift_adj_value << endl;
            break;
        }
    }

}
void Clock2::adj_offset_drift()
{
    ev << "update clock offset" << endl;
    ev << "simTime = "<< SIMTIME_DBL(simTime()) << ", and lastupdatetime = "<< lastupdatetime <<endl;

    ev << "Clock: offset(-) = " << offset << endl;
    offset_valueVec.record(offset);
    ev << "Clock: drift(-) = " << drift << endl;
    drift_valueVec.record(drift);

    //TODO:/*moving filter*/
    //movingfilter();
    //TODO:/*kalma filter*/
    //   kalmanfilter();
    //TODO:¸üÐÂdrift¹À¼Æ¹«Ê½ÖÐµÄ±äÁ¿£¬ÒòÎªÊ±ÖÓ¸üÐÂÊ±¼ÓÁËËÅ·þ

    Tm_previous = Tm;
    ev << "clock_Tm_previous = " << Tm_previous << endl;
    offset_adj_previous = offset_adj_value;
    ev << "offset_adj_value = " << offset_adj_value << endl;
    ev << "drift_adj_value = " << drift_adj_value << endl;
    drift_adj_valueVec.record(drift_adj_value);
    offset_adj_valueVec.record(offset_adj_value);

    offset = offset - offset_adj_value;
    drift = drift - drift_adj_value;

    ev << " offset(+) = " << offset << endl;
    error_offset = offset;
    //error_offset = offset - offset_adj_value;
    error_offsetVec.record(error_offset);

    ev << "drift(+) = " << drift << endl;
    error_drift = drift;
    //error_drift = drift - drift_adj_value;
    error_driftVec.record(error_drift);

    j = j+1;
    if(j > 10)
    {
        error_sync_offset.collect(error_offset);
        error_sync_drift.collect(error_drift);
    }
    //preprocess_offset();
}


void Clock2::preprocess_offset(){
    Packet *pck = new Packet("PROCESSED_OFFSET");
    pck->setPckType(CLOCK);
    pck->setClockType(PROCESSED_OFFSET);
    pck->setData(offset_adj_value);
    ev<<"offset_adj_value="<<offset_adj_value<<endl;
    send(pck,"outclock");
}

void Clock2::movingfilter(){
    //¸üÐÂoffset_adj_value ºÍ drift_adj_value
            offset_adj_value = offset_adj_value*alpha;
            drift_adj_value = drift_adj_value*beta;
           // preprocess_offset();
            ev<<" offset_adj_value= "<< offset_adj_value<<endl;
            ev<<" drift_adj_value= "<<drift_adj_value<<endl;

}

void Clock2::kalmanfilter(){
    double Atr[2][2]={{1,0},{Tsync,1}};//Transposed matrix A µÄ×ªÖÃ¾ØÕó
    double Htr[2][2]={{1,0},{0,1}};//H µÄ×ªÖÃ¾ØÕó
    double Pkbar[2][2];//ºóÑé¹À¼ÆÎó²îÐ­·½²î¾ØÕó
    double xkbar[2][1];//¾­KFÂË²¨ºóµÄoffset¡¢drift
    double ukbar[2][1];//ukbar =B*ukhat
    double ykhat[2][1]={{offset_adj_value},{drift_adj_value}};
    double Kfinv[2][2];
    double x[4];
    double y[8];
    double z[8];
    double temp;
    int r,s,t,m;
    for(r=0;r<=1;r++)
    {
        ev<<"ykhat[r][0]= "<<ykhat[r][0]<<endl;
    }
    //TODO:Ê±¼ä¸üÐÂ·½³Ì
   //TODO:¹«Ê½1 ÊµÏÖ xkbar=A*xkhat+B*ukhat µÄ´úÂë
    //TODO:(1.1) ÊµÏÖxkbar=A*xkhat
    ev<<"time_update function"<<endl;
    ev<<"Function (1.1) : xkbar=A*xkhat"<<endl;
    ev<<"xkhat[0][0]= "<<xkhat[0][0]<<endl;
    ev<<"xkhat[1][0]= "<<xkhat[1][0]<<endl;
    for(r=0,m=0;r<=1;r++)
    {
        for(s=0;s<=1;s++)
        {
        x[m] = A[r][s]*xkhat[s][0];
        ev<<"x[m]= "<<x[m]<<endl;
        m++;
        }
    }
    for(r=0,s=0;r<=1;r++)
    {
      //for(j=0;j<=2;j=j+2)
      xkbar[r][0]=x[s]+x[s+1];
      s=s+2;
      ev<<"xkbar[r][0]= "<<xkbar[r][0]<<endl;
    }
    //TODO:(1.2) ÊµÏÖukbar =B*ukhat
    ev<<"Function (1.2) : ukbar = B*ukhat"<<endl;
    ev<<"ukhat[0][0]= "<<ukhat[0][0]<<endl;
    ev<<"ukhat[1][0]= "<<ukhat[1][0]<<endl;
    for(r=0,m=0;r<=1;r++)
    {
        for(s=0;s<=1;s++)
        {
        x[m] = B[r][s]*ukhat[s][0];
        ev<<"x[m]= "<<x[m]<<endl;
        m++;
        }
    }
    for(r=0,s=0;r<=1;r++)
    {
      //for(j=0;j<=2;j=j+2)
      ukbar[r][0]=x[s]+x[s+1];
      s=s+2;
      ev<<"ukbar[r][0]= "<<ukbar[r][0]<<endl;
    }
    //TODO:(1.3) ÊµÏÖxkbar=xkbar+ukbar
    ev<<"Function (1.3) :xkbar=xkbar+ukbar"<<endl;
    for(r=0;r<=1;r++)
         {
          xkbar[r][0]= xkbar[r][0]+ukbar[r][0];
          ev<<"xkbar[r][0]= "<<xkbar[r][0]<<endl;
         }


//ev<<"xkbar[0][0]= "<<xkbar[0][0]<<endl;
//ev<<"xkbar[1][0]= "<<xkbar[1][0]<<endl;

    //TODo:¹«Ê½2 ÊµÏÖ Pkbar=A*Pk*A'+Q µÄ´úÂë
    //TODO:(2.1)ÊµÏÖPkbar = A*Pk
    ev<<"Function (2) : Pkbar=A*Pk*A'+Q"<<endl;
    ev<<"(2.1) Pkbar = A*Pk:"<<endl;
    for(r=0,m=0;r<=1;r++)
    {
        for(s=0;s<=1;s++)
        {
            for(t=0;t<=1;t++)
            {
             y[m] = A[r][s]*Pk[s][t];
             ev<<"y[m]= "<<y[m]<<endl;
             m++;
            }
        }
    }
  /*  for(r=0;r<=7;r++){
        ev<<"y[r]= "<<y[r]<<endl;
    }*/
    for(r=0,m=0;r<=1;r++)
    {
        for(s=0;s<=1;s++)
        {
            Pkbar[r][s] = y[m]+y[m+2];
            ev<<"y[m]= "<<y[m]<<endl;
            ev<<"y[m+2]= "<<y[m+2]<<endl;
            ev<<"Pkbar[r][s]= "<<Pkbar[r][s]<<endl;
            m = m+1;
            ev<<"m= "<<m<<endl;
        }
        m = m+2;
        ev<<"m= "<<m<<endl;

    }
//TODO:(2.2)ÊµÏÖPkbar = Pkbar*Atr
    ev<<"(2.2) Pkbar = Pkbar*Atr"<<endl;
for(r=0,m=0;r<=1;r++)
   {
    for(s=0;s<=1;s++)
    {
        for(t=0;t<=1;t++)
        {
         z[m] = Pkbar[r][s]*Atr[s][t];
         ev<<"z[m]= "<<z[m]<<endl;
         m++;
        }
    }
   }

   for(r=0,m=0;r<=1;r++)
   {
    for(s=0;s<=1;s++)
    {
        Pkbar[r][s] = z[m]+z[m+2];
        ev<<"z[m]= "<<z[m]<<endl;
        ev<<"z[m+2]= "<<z[m+2]<<endl;
        ev<<"Pkbar[r][s]= "<<Pkbar[r][s]<<endl;
        m = m+1;
        ev<<"m= "<<m<<endl;
    }
    m = m+2;
    ev<<"m= "<<m<<endl;

   }
  //TODO:(2.3)ÊµÏÖPkbar = Pkbar+Q
   ev<<"(2.3) Pkbar = Pkbar+Q"<<endl;
   for(r=0;r<=1;r++)
   {
       for (s=0;s<=1;s++)
       {
           Pkbar[r][s] = Pkbar[r][s]+Q[r][s];
           ev<<"Pkbar[r][s]= "<<Pkbar[r][s]<<endl;
       }
   }

   //TODo: ²âÁ¿¸üÐÂ·½³Ì
   //TODo: ¹«Ê½3  ÊµÏÖKf=Pkbar*H'/(H*Pkbar*H'+R)
   //(3.1)ÊµÏÖKf=Pkbar*H'
   ev<<"measurement_update function:"<<endl;
   ev<<"Function (3) : Kf=Pkbar*H'/(H*Pkbar*H'+R)"<<endl;
   ev<<"(3.1) Kf=Pkbar*H':"<<endl;
   for(r=0,m=0;r<=1;r++)
         {
            for(s=0;s<=1;s++)
            {
                for(t=0;t<=1;t++)
                {
                 z[m] = Pkbar[r][s]*Htr[s][t];
                 ev<<"z[m]= "<<z[m]<<endl;
                 m++;
                }
            }
         }

         for(r=0,m=0;r<=1;r++)
         {
            for(s=0;s<=1;s++)
            {
                Kf[r][s] = z[m]+z[m+2];
                ev<<"z[m]= "<<z[m]<<endl;
                ev<<"z[m+2]= "<<z[m+2]<<endl;
                ev<<"Kf[r][s]= "<<Kf[r][s]<<endl;
                m = m+1;
                ev<<"m= "<<m<<endl;
            }
            m = m+2;
            ev<<"m= "<<m<<endl;

         }

   //TODo:(3.2)ÊµÏÖKfinv=H*Pkbar*H'+R
   //TODo:(3.2.1)ÊµÏÖKfinv = H*Pkbar
   ev<<"(3.2)  Kfinv=I/H*Pkbar*H'+R:"<<endl;
   ev<<"(3.2.1) Kfinv = H*Pkbar:"<<endl;
       for(r=0,m=0;r<=1;r++)
       {
        for(s=0;s<=1;s++)
        {
            for(t=0;t<=1;t++)
            {
             y[m] = H[r][s]*Pkbar[s][t];
             ev<<"y[m]= "<<y[m]<<endl;
             m++;
            }
        }
       }

       for(r=0,m=0;r<=1;r++)
       {
        for(s=0;s<=1;s++)
        {
            Kfinv[r][s] = y[m]+y[m+2];
            ev<<"y[m]= "<<y[m]<<endl;
            ev<<"y[m+2]= "<<y[m+2]<<endl;
            ev<<"Kfinv[r][s]= "<<Kfinv[r][s]<<endl;
            m = m+1;
            ev<<"m= "<<m<<endl;
        }
        m = m+2;
        ev<<"m= "<<m<<endl;

       }


   //TODo:(3.2.2)ÊµÏÖKf = Kf*H'
   ev<<"(3.2.2) Kfinv = Kfinv*H':"<<endl;
   for(r=0,m=0;r<=1;r++)
      {
        for(s=0;s<=1;s++)
        {
            for(t=0;t<=1;t++)
            {
             z[m] = Kfinv[r][s]*Htr[s][t];
             ev<<"z[m]= "<<z[m]<<endl;
             m++;
            }
        }
      }

      for(r=0,m=0;r<=1;r++)
      {
        for(s=0;s<=1;s++)
        {
            Kfinv[r][s] = z[m]+z[m+2];
            ev<<"z[m]= "<<z[m]<<endl;
            ev<<"z[m+2]= "<<z[m+2]<<endl;
            ev<<"Kfinv[r][s]= "<<Kfinv[r][s]<<endl;
            m = m+1;
            ev<<"m= "<<m<<endl;
        }
        m = m+2;
        ev<<"m= "<<m<<endl;

      }
     //TODo:(3.2.3)ÊµÏÖKfinv = Kfinv+R
      ev<<"(3.2.3) Kfinv = Kfinv+R"<<endl;
      for(r=0;r<=1;r++)
      {
       for (s=0;s<=1;s++)
       {
           Kfinv[r][s] = Kfinv[r][s]+R[r][s];
           ev<<"Kfinv[r][s]= "<<Kfinv[r][s]<<endl;
       }
      }

    //TODo:(3.2.4)ÇóKfinvµÄÄæ¾ØÕó£¬¼´Kfinv=I/Kfinv = I/(H*Pkbar*H'+R)
     ev<<"(32.4) Kfinv = I/Kfinv:"<<endl;
     temp = Kfinv[0][0]*Kfinv[1][1]-Kfinv[0][1]*Kfinv[1][0];
     ev<<"temp1= "<<temp<<endl;
     temp = 1/temp;
     ev<<"temp2= "<<temp<<endl;
     x[0]=Kfinv[1][1];
     x[1]=-Kfinv[0][1];
     x[2]=-Kfinv[1][0];
     x[3]=Kfinv[0][0];
     for(r=0,m=0;r<=1;r++)
     {
         for(s=0;s<=1;s++)
         {
             Kfinv[r][s]=temp*x[m];
             ev<<"Kfinv[r][s]= "<<Kfinv[r][s]<<endl;
             m++;
         }
     }
    //TODo:(3.3) ÊµÏÖKf=Kf*Kfinv,¼´Kf=Pkbar*H'/(H*Pkbar*H'+R)
     ev<<"(3.3) three: Kf=Kf*Kfinv"<<endl;
     for(r=0,m=0;r<=1;r++)
        {
            for(s=0;s<=1;s++)
            {
                for(t=0;t<=1;t++)
                {
                 z[m] = Kf[r][s]*Kfinv[s][t];
                 ev<<"z[m]= "<<z[m]<<endl;
                 m++;
                }
            }
        }

        for(r=0,m=0;r<=1;r++)
        {
            for(s=0;s<=1;s++)
            {
                Kf[r][s] = z[m]+z[m+2];
                ev<<"z[m]= "<<z[m]<<endl;
                ev<<"z[m+2]= "<<z[m+2]<<endl;
                ev<<"Kf[r][s]= "<<Kf[r][s]<<endl;
                m = m+1;
                ev<<"m= "<<m<<endl;
            }
            m = m+2;
            ev<<"m= "<<m<<endl;

        }


    //TOdo:¹«Ê½4 ÊµÏÖxkhat=xkbar+Kf*(y-xkbar)
     ev<<"Function (4) :  xkhat=xkbar+Kf*(ykhat-xkbar)"<<endl;
     ev<<"(4.1) ykhat=ykhat-xkbar"<<endl;
           for(r=0;r<=1;r++)
           {
                   ykhat[r][0] = ykhat[r][0]- xkbar[r][0];
                   ev<<" ykhat[r][0]= "<< ykhat[r][0]<<endl;
           }

      ev<<"(4.2) xkhat=Kf*ykhat=Kf*(ykhat-xkbar)"<<endl;
      for(r=0,m=0;r<=1;r++)
      {
        for(s=0;s<=1;s++)
        {
          x[m] = Kf[r][s]*ykhat[s][0];
          ev<<"x[m]= "<<x[m]<<endl;
          m++;
        }
      }
      for(r=0,s=0;r<=1;r++)
      {
        //for(j=0;j<=2;j=j+2)
        xkhat[r][0]=x[s]+x[s+1];
        ev<<"xkhat[r][0]= "<<xkhat[r][0]<<endl;
        s=s+2;
      }

      ev<<"(4.3) xkhat= xkbar+xkhat"<<endl;
      for(r=0;r<=1;r++)
      {
          xkhat[r][0]= xkbar[r][0]+xkhat[r][0];
          ev<<"xkhat[r][0]= "<<xkhat[r][0]<<endl;
      }

     //TODo:¹«Ê½5 ÊµÏÖPk=Pkbar-Kf*H*Pkbar
     // (5.1)ÊµÏÖ Pk=Kf*H*Pkbar
      ev<<"Function (5) :  Pk=Pkbar-Kf*H*Pkbar"<<endl;
      ev<<"(5.1)  Pk=Kf*H*Pkbar"<<endl;
      ev<<"(5.1.1) Pk=Kf*H"<<endl;
      for(r=0,m=0;r<=1;r++)
              {
                for(s=0;s<=1;s++)
                {
                    for(t=0;t<=1;t++)
                    {
                     z[m] = Kf[r][s]*H[s][t];
                     ev<<"z[m]= "<<z[m]<<endl;
                     m++;
                    }
                }
              }

              for(r=0,m=0;r<=1;r++)
              {
                for(s=0;s<=1;s++)
                {
                    Pk[r][s] = z[m]+z[m+2];
                    ev<<"z[m]= "<<z[m]<<endl;
                    ev<<"z[m+2]= "<<z[m+2]<<endl;
                    ev<<"Pk[r][s]= "<<Pk[r][s]<<endl;
                    m = m+1;
                    ev<<"m= "<<m<<endl;
                }
                m = m+2;
                ev<<"m= "<<m<<endl;

              }

           ev<<"(5.1.2) Pk=Pk*Pkbar"<<endl;
           for(r=0,m=0;r<=1;r++)
                   {
                    for(s=0;s<=1;s++)
                    {
                        for(t=0;t<=1;t++)
                        {
                         z[m] = Pk[r][s]*Pkbar[s][t];
                         ev<<"z[m]= "<<z[m]<<endl;
                         m++;
                        }
                    }
                   }
                 /*  for(r=0;r<=7;r++){
                    ev<<"z[r]= "<<z[r]<<endl;
                   }*/
                   for(r=0,m=0;r<=1;r++)
                   {
                    for(s=0;s<=1;s++)
                    {
                        Pk[r][s] = z[m]+z[m+2];
                        ev<<"z[m]= "<<z[m]<<endl;
                        ev<<"z[m+2]= "<<z[m+2]<<endl;
                        ev<<"Pk[r][s]= "<<Pk[r][s]<<endl;
                        m = m+1;
                        ev<<"m= "<<m<<endl;
                    }
                    m = m+2;
                    ev<<"m= "<<m<<endl;

                   }

        //TOdo:(5.2)ÊµÏÖPk=Pkbar-Pk,¼´Pk=Pkbar-Kf*H*Pkbar
        ev<<"(5.2):  Pk=Pkbar-Pk "<<endl;
        for(r=0;r<=1;r++)
           {
               for (s=0;s<=1;s++)
               {
                   Pk[r][s] = Pkbar[r][s]-Pk[r][s];
                   ev<<"Pk[r][s]= "<<Pk[r][s]<<endl;
               }
           }

        ev<<"ukhat[0][0]= "<<ukhat[0][0]<<endl;
        ev<<"ukhat[1][0]= "<<ukhat[1][0]<<endl;
        //¸üÐÂoffset_adj_value ºÍ drift_adj_value
        offset_adj_value = xkhat[0][0];
        drift_adj_value = xkhat[1][0];
        ev<<" offset_adj_value= "<< offset_adj_value<<endl;
        ev<<" drift_adj_value= "<<drift_adj_value<<endl;
        //¸üÐÂukhat[2][1]
        ukhat[0][0]= offset_adj_value;
        ukhat[1][0]= drift_adj_value;
}




/*void Clock2::adjtimex(double value[2]){
    ev << "---------------------------------" << endl;
            ev << "CLOCK : AGGIORNAMENTO OFFSET" << endl;
            ev << "CLOCK : offset- = " << offset<<endl;
            ev<<"simTime="<<SIMTIME_DBL(simTime())<<" lastupdatetime="<<lastupdatetime<<endl;
            offset_valueVec.record(offset);
            ev<<"value[0]= "<<value[0]<<endl;
            offset = offset - value[0];
            ev << " offset+ = " << offset << endl;
            error_offset = offset;
            error_offsetVec.record(error_offset);


            ev<<"drift- ="<<drift<<endl;
            drift_valueVec.record(drift);
            ev<<"value[1]= "<<value[1]<<endl;
            drift = drift - value[1];
            ev<<"drift+ ="<<drift<<endl;
            error_drift = drift;
            error_driftVec.record(error_drift);

            j= j+1;
            if(j>=10){
            error_sync_offset.collect(error_offset);
            error_sync_drift.collect(error_drift);
            }

}*/

void Clock2::finish(){
    //recordScalar("measure Uncertainty",softclock);
    error_sync_drift.getMin();
    error_sync_drift.getMax();
    error_sync_offset.getMin();
    error_sync_offset.getMax();
    //ev<<"error_sync_drift.Min= "<<error_sync_drift.getMin()<<endl;
    //ev<<"error_sync_drift.Max= "<<error_sync_drift.getMax()<<endl;
    //ev<<"error_sync_offset.Min= "<<error_sync_offset.getMin()<<endl;
    //ev<<"error_sync_offset.Max= "<<error_sync_offset.getMax()<<endl;
    //TODO:
    recordScalar("sigma3",sigma3);
    //recordScalar("alpha",alpha);
    //recordScalar("beta",beta);
    recordScalar("drift_mean",driftStd.getMean());
    recordScalar("drift_std",driftStd.getStddev());
    recordScalar("offset_mean",offsetStd.getMean());
    recordScalar("offset_std",offsetStd.getStddev());
    recordScalar("error_drift_mean",error_sync_drift.getMean());
    recordScalar("error_drift_std",error_sync_drift.getStddev());
    recordScalar("error_offset_mean",error_sync_offset.getMean());
    recordScalar("error_offset_std",error_sync_offset.getStddev());
    //closefile();
}


void Clock2::updateDisplay(){
    char buf[100];
    sprintf(buf, "offset [msec]: %3.2f   \ndrift [ppm]: %3.2f \norigine: %3.2f",
        offset,drift*1E6,lastupdatetime);
    getDisplayString().setTagArg("t",0,buf);
}

/*void Clock2::openfile(){
    ev << "CLOCK: ---- APERTURA FILE ----\n";
    outFile.open("clockdata.txt");
    if(!outFile){
        ev << "Il file non puo` essere aperto\n";
    }
    else{
        ev << "Il file e` stato aperto con successo\n";
    }
}

void Clock2::closefile(){
    ev << "CLOCK: ---- CHIUSURA FILE ----\n";
    outFile.close();
}*/

void Clock2::adjustThreshold()
{
    ev << "Clock: adjust threshold of clock... "<< endl;

    // ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - getPCOTimestamp() - ScheduleOffset*NodeId - delay;
    ClockOffset = ReceivedPulseTime - numPulse*FrameDuration - getPCOTimestamp() - delay - ScheduleOffset - slotDuration*NodeId;

    ThresholdOffset = ClockOffset;
    ev << "Clock: the threshold offset is "<< ThresholdOffset << ", and the clock offset is " << ClockOffset << endl;

    ThresholdAdjustValue = AdjustParameter*ThresholdOffset;
    // ThresholdAdjustValue = (ThresholdOffset + ThresholdAdjustValue)/2;
    ev << "Clock: based on the threshold adjustment value: "<< ThresholdAdjustValue << ", the RegisterThreshold change from " << RegisterThreshold;
    RegisterThreshold = RegisterThreshold - ThresholdAdjustValue;
    ev << " to " << RegisterThreshold << endl;
    ev << "Clock: adjust threshold of clock is finished "<< endl;
}

int Clock2::getnumPulse()
{
    ev << "Clock: the returned 'numPulse' is " << numPulse << endl;
    return numPulse;
}

void Clock2::generateSYNC()
{
    EV << "Clock: PCO time reaches threshold, generate a SYNC packet \n";

    PtpPkt *pck = new PtpPkt("SYNC");
    pck->setPtpType(SYNC);
    pck->setByteLength(40); // SYNC_BYTE = 40

    /*
    pck->setSource(myAddress);
    pck->setDestination(PTP_BROADCAST_ADDR);

    pck->setData(SIMTIME_DBL(simTime()));
    pck->setTsTx(SIMTIME_DBL(simTime())); // set transmission time stamp ts1 on SYNC

    // set SrcAddr, DestAddr with LAddress::L3Type values for MiXiM
    pck->setSrcAddr( LAddress::L3Type(myAddress));
    pck->setDestAddr(LAddress::L3BROADCAST);

    // set the control info to tell the network layer (layer 3) address
    NetwControlInfo::setControlInfo(pck, LAddress::L3BROADCAST );
    */

    EV << "Clock send SYNC packet to Core module" << endl;
    send(pck,"outclock");

}

double Clock2::setReceivedTime(double value)
{
    ReceivedPulseTime = value;
    return ReceivedPulseTime;
}

cModule *Clock2::findHost(void)
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

