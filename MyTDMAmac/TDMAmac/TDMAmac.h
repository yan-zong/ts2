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


#ifndef TDMAMAC_H_
#define TDMAMAC_H_

#include <omnetpp.h>
#include <BaseMacLayer.h>
#include <SimpleAddress.h>
#include <DroppedPacket.h> //Not used yet !aaks
#include <Clock2.h>

class TDMAMacPkt; // TDMA Mac packet definition !aaks look at TDMAMacPkt.msg

class MIXIM_API TDMAmac : public BaseMacLayer
{
    /* Parts copied from LMAC definition. !aaks */

    private:
        /* @brief Copy constructor is not allowed. #LMAC */
        TDMAmac(const TDMAmac&);

        /* @brief Assignment operator is not allowed. #LMAC */
        TDMAmac& operator=(const TDMAmac&);

        Clock2 *pClock;

    /*
     * New variables apart from LMAC definition are:
     * transmitSlot - An array that mentions which node transmits in a particular slot.
     * reciveSlot - An array that mentions which node receives in the slot. Mainly represented
     * by nodeID. For all receive a non-existent nodeID to be used.
     * Still in its basic form need better methods for static slot representation.
     * nextEvent - A variable used to store the time for next event calculated
     * !aaks
     */

    public:TDMAmac():BaseMacLayer()
        , slotDuration(0)
        , numSlots(0)
        , bitrate(0)
        , macPktQueue()
        , txPower(0)
    {}

       /* Module destructor #LMAC */
       virtual ~TDMAmac();

       /* MAC initialization #LMAC and BaseMacLayer definition !aaks */
       virtual void initialize(int stage);

       /* @brief Delete all dynamically allocated objects of the module #LMAC */
       virtual void finish();

       /* Handles packets received from the Physical Layer. #LMAC */
       virtual void handleLowerMsg(cMessage* msg);

       /*
        * Handles packets from the upper layer (Network/Application) and prepares
        * them to be sent down. #LMAC
        */
       virtual void handleUpperMsg(cMessage* msg);

       /* @brief Handle self messages such as timer, to move through slots #LMAC !aaks */

       virtual void handleSelfMsg(cMessage*);

       /*
        * Handles and forwards the control messages received
        * from the physical layer. #LMAC
        */
       virtual void handleLowerControl(cMessage* msg);

       /*
        * Encapsulates the packet from the upper layer and
        * creates and attaches the signal to it.
        */
       virtual macpkt_ptr_t encapsMsg(cPacket* Pkt);

protected:
    bool debug, stats, trace;

    /* Mac packet queue to store incoming packets from upper layer !aaks */

    typedef std::list<TDMAMacPkt*> MacPktQueue;

    /* @brief Duration of a slot #LMAC */
    double slotDuration;
    /* @brief how many slots are there #LMAC */
    int numSlots;

    /*@brief length of the queue #LMAC */
    unsigned queueLength;

    /* Self messages for TDMA slot timer callbacks */
    cMessage* delayTimer;

    int numNodes;

    /* @brief the bit rate at which we transmit #LMAC */
    double bitrate;

    bool gateway;

    /* @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission #LMAC !aaks */
    MacPktQueue macPktQueue;

    /* @brief Inspect reasons for dropped packets #LMAC !aaks not used yet */
    DroppedPacket droppedPacket;

    /* @brief Transmission power of the node #LMAC */
    double txPower;

    /* @brief Internal function to attach a signal to the packet #LMAC */
    void attachSignal(macpkt_ptr_t macPkt);

    /*Stats**/
    cOutVector vqLength;
    cStdDev qLength;
    int droppedPackets;

    int MyID = 0;

    double ClockTime;
    double ClockTimeOffset;
    double ScheduleTimeOffset;
    double FrameTime;
    double GuardTime;
};

#endif /* TDMAMAC_H_ */
