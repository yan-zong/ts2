//***************************************************************************
// * File:        This file is part of TS2.
// * Created on:  29 Jan 2014
// * Author:      Yiwen Huang, Xuweu Dai  (x.dai at ieee.org)
// *
// * Copyright:   (C) 2014 Southwest University, Chongqing, China.
// *
// *              TS2 is free software; you can redistribute it  and/or modify
// *              it under the terms of the GNU General Public License as published
// *              by the Free Software Foundation; either  either version 3 of
// *              the License, or (at your option) any later version.
// *
// *              TS2 is distributed in the hope that it will be useful,
// *                  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *                  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *                  GNU General Public License for more details.
// *
// * Credit:      Yiwen Huang, Taihua Li
// * Funding:     This work was partially financed by the National Science Foundation China
// %              _
// %  \/\ /\ /   /  * _  '
// % _/\ \/\/ __/__.'(_|_|_
// **************************************************************************/

#include "Clock2.h"
#include "Constant.h"


Define_Module(Clock2);

void Clock2::initialize(){
    // ---------------------------------------------------------------------------
    // Inizializzazione variabile per il salvataggio dei dati in uscita.
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
    // ---------------------------------------------------------------------------
    // Lettura parametri di ingresso.
    // ---------------------------------------------------------------------------
    offset = par("offset");
    drift =  par("drift");
    sim_time_limit = par("sim_time_limit");
    error_drift = offset;
    error_offset =drift;
    sigma1  = par("sigma1");
    sigma2 = par("sigma2");
    sigma3 = par("sigma3");
    u3 = par("u3");
    Tcamp  = par("Tcamp");
    Tsync = par("Tsync");
    alpha=par("alpha");
    beta=par("beta");
    // ---------------------------------------------------------------------------
    // Lettura parametri di ingresso.
    // ---------------------------------------------------------------------------
    lastupdatetime = SIMTIME_DBL(simTime());
    phyclock = softclock = offset;
    drift_previous =drift;
    offset_previous=offset;
    i = 0;
    j = 0;
    delta_drift = delta_offset = 0;
    k = int(sim_time_limit/Tcamp);
    Tm = Tm_previous =0;
    offset_adj_previous=0;
    /*kalman filter parameter Initialize*/

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

    //openfile();
    // ---------------------------------------------------------------------------
    // Inizializzazione del timer. Il timer viene ripristinato con un tempo fisso
    // pari a 1.0 secondo. Viene utilizzato per campionare in intervalli di
    // durata costante il clock di sistema. I valori campionati vengono salvati
    // nella vraibile di uscita timestampVec.
    // ---------------------------------------------------------------------------
    if(ev.isGUI()){updateDisplay();}
    //TODO::??131
    //recordResult();
    delta_driftVec.record(delta_drift);
    delta_offsetVec.record(delta_offset);
    scheduleAt(simTime() + Tcamp,new cMessage("CLTimer"));
}

void Clock2::handleMessage(cMessage *msg){
    if(msg->isSelfMessage()){
    // ---------------------------------------------------------------------------
    // Timer. Viene salvato il valore del clock in uscita, ottenuto mediante la
    // funzione interna getTimestamp(). Viene ripristinato il timer a 1.0 s.
    // ---------------------------------------------------------------------------
        //TODO:??
        Phyclockupdate();
        i = i+1;
        ev<<"i= "<<i<<endl;
        ev<<"k="<<k<<endl;
        /*加if判断语句，在sim_time_limit/Tcamp较小时可以记录较多数据*/
        if(i%10==0){
                    ev<<"count delta_drfit and delta_offset:"<<endl;
                     delta_drift = drift - drift_previous;
                     delta_offset = offset - offset_previous;
                     if(k >= 9999999){
                         ev<<"compare 1 success!"<<endl;
                         if(i%100==0){
                         ev<<"Larger amount of data,record delta_offset and delta_drift."<<endl;
                         delta_driftVec.record(delta_drift);
                         delta_offsetVec.record(delta_offset);
                         }
                     }
                     else{
                         ev<<"less amount of data，record delta_offset and delta_drift."<<endl;
                         delta_driftVec.record(delta_drift);
                         delta_offsetVec.record(delta_offset);
                     }
                     drift_previous = drift;
                     offset_previous = offset;
                }
        if(k>=9999999){
            ev<<"compare 2 success!"<<endl;
            if((i>10)&&(i%100==0)){
            ev<<"Larger amount of data,record offset and drift of updatePhyclock"<<endl;
            recordResult();
            driftStd.collect(drift);
            offsetStd.collect(offset);
            }
         }
        else{
            ev<<"Less amount of data,record offset and drift of updatePhyclock"<<endl;
            recordResult();
            //TODO:统计drift和offset的值
            driftStd.collect(drift);
            offsetStd.collect(offset);
        }

        scheduleAt(simTime()+ Tcamp,new cMessage("CLTimer"));
    }else{
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
    if(ev.isGUI()){updateDisplay();}
}
// TODo:adding Phyclockupdate()
double Clock2::Phyclockupdate(){
    ev<<"update Phyclock&offset:"<<endl;
    noise2 =  normal(0,sigma2,1);
    offset = offset + drift*(SIMTIME_DBL(simTime())-lastupdatetime)+ noise2;
    //offset = (drift+ noise1)*(SIMTIME_DBL(simTime())-lastupdatetime)+ noise2;
    noise1 =  normal(0,sigma1,1);
    drift = drift + noise1;
    //drift = drift;
    phyclock = offset + SIMTIME_DBL(simTime());
    lastupdatetime = SIMTIME_DBL(simTime());
    ev<<"offset="<<offset<<endl;
    ev<<"simTime=lastupdatetime="<<SIMTIME_DBL(simTime())<<endl;
    return phyclock;
}
void Clock2::recordResult(){
    driftVec.record(drift);
    offsetVec.record(offset);
    update_numberVec.record(i);
    noise1Vec.record(noise1);
    noise2Vec.record(noise2);

}

double Clock2::getTimestamp(){
    //TODO:Modify
    ev<<"getTimestamp:"<<endl;
    ev<<"simTime="<<SIMTIME_DBL(simTime())<<" lastupdatetime="<<lastupdatetime<<endl;
    double clock = offset + drift*(SIMTIME_DBL(simTime())-lastupdatetime) + SIMTIME_DBL(simTime());
    ev<<"clock= "<<clock<<endl;
    noise3 = normal(u3,sigma3);
    ev<<"noise3= "<<noise3<<endl;
    noise3Vec.record(noise3);
    softclock = clock ;//+ noise3;
    ev<<"softclock= "<<softclock<<endl;
    softclockVec.record(softclock);
    //ev.printf("phyclock_float=%8f",phyclock_float);
    return softclock;
}


void Clock2::adjtimex(double value, int type){
    switch(type){
    case 0: //offset
        /*noise3 = normal(0,sigma3);
        ev<<"noise3= "<<noise3<<endl;
        noise3Vec.record(noise3);
        offset_adj_value = value + noise3;*/
        offset_adj_value = value;
        ev<<"offset_value= "<<value<<endl;
        ev<<"offset_adj_value = "<<offset_adj_value<<endl;

        break;
    case 1: //drift
        ev<<"clock_Tm_previous= "<<Tm_previous<<endl;
        ev<<"clock_Tm - clock_Tm_previous= "<<Tm - Tm_previous<<endl;
        drift_adj_value = value+ offset_adj_previous/(Tm - Tm_previous);
        //drift_adj_value = offset_adj_value/Tsync;
        ev<<"drift_value= "<<value<<endl;
        ev<<"drift_adj_value = "<<drift_adj_value<<endl;
        break;
    }

}
void Clock2::adj_offset_drift(){
                ev << "---------------------------------" << endl;
                ev << "CLOCK : AGGIORNAMENTO OFFSET" << endl;
                ev<<"simTime="<<SIMTIME_DBL(simTime())<<" lastupdatetime="<<lastupdatetime<<endl;

                ev << "CLOCK : offset- = " << offset<<endl;
                offset_valueVec.record(offset);
                ev<<"drift- ="<<drift<<endl;
                drift_valueVec.record(drift);
                //TODO:/*moving filter*/
                //movingfilter();
                //TODO:/*kalma filter*/
             //   kalmanfilter();
                //TODO:更新drift估计公式中的变量，因为时钟更新时加了伺服
                Tm_previous=Tm;
                ev<<"clock_Tm_previous= "<<Tm_previous<<endl;
                offset_adj_previous = offset_adj_value;
                ev<<"offset_adj_value = "<<offset_adj_value<<endl;
                ev<<"drift_adj_value = "<<drift_adj_value<<endl;
                drift_adj_valueVec.record(drift_adj_value);
                offset_adj_valueVec.record(offset_adj_value);
                offset = offset - offset_adj_value;
                drift = drift - drift_adj_value;

                ev << " offset+ = " << offset << endl;
                error_offset = offset;
                //error_offset = offset - offset_adj_value;
                error_offsetVec.record(error_offset);

                ev<<"drift+ ="<<drift<<endl;
                error_drift = drift;
                //error_drift = drift - drift_adj_value;
                error_driftVec.record(error_drift);

                j= j+1;
                if(j>10){
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
    //更新offset_adj_value 和 drift_adj_value
            offset_adj_value = offset_adj_value*alpha;
            drift_adj_value = drift_adj_value*beta;
           // preprocess_offset();
            ev<<" offset_adj_value= "<< offset_adj_value<<endl;
            ev<<" drift_adj_value= "<<drift_adj_value<<endl;

}

void Clock2::kalmanfilter(){
    double Atr[2][2]={{1,0},{Tsync,1}};//Transposed matrix A 的转置矩阵
    double Htr[2][2]={{1,0},{0,1}};//H 的转置矩阵
    double Pkbar[2][2];//后验估计误差协方差矩阵
    double xkbar[2][1];//经KF滤波后的offset、drift
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
    //TODO:时间更新方程
   //TODO:公式1 实现 xkbar=A*xkhat+B*ukhat 的代码
    //TODO:(1.1) 实现xkbar=A*xkhat
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
    //TODO:(1.2) 实现ukbar =B*ukhat
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
    //TODO:(1.3) 实现xkbar=xkbar+ukbar
    ev<<"Function (1.3) :xkbar=xkbar+ukbar"<<endl;
    for(r=0;r<=1;r++)
         {
          xkbar[r][0]= xkbar[r][0]+ukbar[r][0];
          ev<<"xkbar[r][0]= "<<xkbar[r][0]<<endl;
         }


//ev<<"xkbar[0][0]= "<<xkbar[0][0]<<endl;
//ev<<"xkbar[1][0]= "<<xkbar[1][0]<<endl;

    //TODo:公式2 实现 Pkbar=A*Pk*A'+Q 的代码
    //TODO:(2.1)实现Pkbar = A*Pk
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
//TODO:(2.2)实现Pkbar = Pkbar*Atr
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
  //TODO:(2.3)实现Pkbar = Pkbar+Q
   ev<<"(2.3) Pkbar = Pkbar+Q"<<endl;
   for(r=0;r<=1;r++)
   {
       for (s=0;s<=1;s++)
       {
           Pkbar[r][s] = Pkbar[r][s]+Q[r][s];
           ev<<"Pkbar[r][s]= "<<Pkbar[r][s]<<endl;
       }
   }

   //TODo: 测量更新方程
   //TODo: 公式3  实现Kf=Pkbar*H'/(H*Pkbar*H'+R)
   //(3.1)实现Kf=Pkbar*H'
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

   //TODo:(3.2)实现Kfinv=H*Pkbar*H'+R
   //TODo:(3.2.1)实现Kfinv = H*Pkbar
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


   //TODo:(3.2.2)实现Kf = Kf*H'
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
     //TODo:(3.2.3)实现Kfinv = Kfinv+R
      ev<<"(3.2.3) Kfinv = Kfinv+R"<<endl;
      for(r=0;r<=1;r++)
      {
       for (s=0;s<=1;s++)
       {
           Kfinv[r][s] = Kfinv[r][s]+R[r][s];
           ev<<"Kfinv[r][s]= "<<Kfinv[r][s]<<endl;
       }
      }

    //TODo:(3.2.4)求Kfinv的逆矩阵，即Kfinv=I/Kfinv = I/(H*Pkbar*H'+R)
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
    //TODo:(3.3) 实现Kf=Kf*Kfinv,即Kf=Pkbar*H'/(H*Pkbar*H'+R)
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


    //TOdo:公式4 实现xkhat=xkbar+Kf*(y-xkbar)
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

     //TODo:公式5 实现Pk=Pkbar-Kf*H*Pkbar
     // (5.1)实现 Pk=Kf*H*Pkbar
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

        //TOdo:(5.2)实现Pk=Pkbar-Pk,即Pk=Pkbar-Kf*H*Pkbar
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
        //更新offset_adj_value 和 drift_adj_value
        offset_adj_value = xkhat[0][0];
        drift_adj_value = xkhat[1][0];
        ev<<" offset_adj_value= "<< offset_adj_value<<endl;
        ev<<" drift_adj_value= "<<drift_adj_value<<endl;
        //更新ukhat[2][1]
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
