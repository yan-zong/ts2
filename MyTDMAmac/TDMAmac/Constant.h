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

/* Constants for simulation */

#define MAX_SLAVE 100

// in omnetpp.ini
// phy.headerLength = 8 x 6 bit (6 bytes)
// mac.headerLength = 8 x 13 bit (13 bytes)
// netwl.headerLength = 0 bit (0 bytes)

/* Packet length  */
#define SYNC_BYTE 40

/** @brief Broadcast and Null address for packet
 *       Since PCO address follows the same as the L3 address ("SimpleAddress.h")
 *       we use the same definition as L3 address
 *       -1   for broadcast address
 *       0    for null address
 * */
#define PTP_BROADCAST_ADDR -1
#define PTP_NULL_ADDR 0

/** @brief Types for packets PtpPkt.ptpType */
enum PtpPacket_types
{
    SYNC = 0,
    REG = 1,
    REGREPLY = 2,
    REGRELAY = 3,
    DREQ = 4,
    DRES = 5
};

/** @brief Values for PtpPkt.pckType
 * OTHER for packet with name "REGISTER"*/
enum
{
    PTP = 0,
    OTHER = 1,
    CLOCK = 2
};

/* Types of events managed by the node manager */
enum
{
    CICLICO = 0,
    ACICLICO = 1
};

