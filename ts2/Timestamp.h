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

#ifndef __TSIM_PTPTIMESTMP_H_
#define __TSIM_PTPTIMESTMP_H_

#include <omnetpp.h>
#include "Clock2.h"
#include "Slave.h"

/**
 * TODO - Generated class
 */
class Timestamp : public cSimpleModule
{
    private:
     bool useGlobalRefClock;
     Clock2 *pClk;
     Slave *pNode;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    cModule *findHost(void);
};

#endif
