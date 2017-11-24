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

// the configuration of SYNC packet, modeling the Pulse of PCO
// phy.headerLength = 8 x 6 bit (6 bytes)
// mac.headerLength = 8 x 13 bit (13 bytes)
// netwl.headerLength = 0 bit (0 bytes)
// timestamp.Length = 8 x 2 bit (2 bytes)

/* Packet length */
#define TIMESTAMP_BYTE 2

/* Broadcast and Null address for packet
 * Since PCO address follows the same as the L3 address ("SimpleAddress.h")
 * we use the same definition as L3 address
 * -1   for broadcast address
 * 0    for null address
 */
#define PACKET_BROADCAST_ADDR -1
#define PACKET_NULL_ADDR 0

/* Types for packets Packet.packetType */
enum packetTypes
{
    REG = 0,    // the register packet from the master node
    REGRELAY = 1,   // the register packet from the relay node
    REGREPLY = 2,
    SYNC = 3,
    DREQ = 4,
    DRES = 5
};

/* Values for Packet.packetType
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

