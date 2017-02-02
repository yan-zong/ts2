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

#ifndef __TSIM_PTPTIMESTMP_H_
#define __TSIM_PTPTIMESTMP_H_

#include <omnetpp.h>
#include "Clock2.h"
#include "Node.h"

/**
 * TODO - Generated class
 */
class PtpTimeStmp : public cSimpleModule
{
    private:
     bool useGlobalRefClock;
     Clock2 *pClk;
     Node *pNode;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    cModule *findHost(void);
};

#endif
