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

#ifndef PTPMASTER_H_
#define PTPMASTER_H_

/**
 * @brief A PTP master class
 *
 * This module provides basic operation
 * of a PTP master working at application layer
 *
 * @author Xuewu Dai
 */

#include <string.h>
#include <omnetpp.h>
#include "PtpPkt_m.h"
#include "Event_m.h"
#include "Constant.h"

class PtpMaster : public cSimpleModule
{
private:
    const char *name;
    int address;
    double Tsync;

protected:
    /** @brief gate id
     *  only data gates are used*/
    /*@{*/
    int upperGateIn;
    int upperGateOut;
    int lowerGateIn;
    int lowerGateOut;
    int inclock;
    // int outclock;
    /*@}*/

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    cModule *findHost(void);

private:
    void handleSelfMessage(cMessage *msg);
    void handleSlaveMessage(PtpPkt *msg);

};

#endif /* PTPMASTER_H_ */
