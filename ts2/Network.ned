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

package ts2;

import org.mixim.base.connectionManager.IConnectionManager;
import org.mixim.base.modules.IWorldUtility;
import org.mixim.examples.ieee802154Narrow.WorldUtilityStats;

// base network 
network MyBaseNetwork
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        // **.mobility.constraintAreaMinX = default(0m);
        // **.mobility.constraintAreaMinY = default(0m);
        // **.mobility.constraintAreaMinZ = default(0m);
        // **.mobility.constraintAreaMaxX = default(playgroundSizeX);
        // **.mobility.constraintAreaMaxY = default(playgroundSizeY);
        // **.mobility.constraintAreaMaxZ = default(playgroundSizeZ);
        string cmType = default("org.mixim.base.connectionManager.ConnectionManager"); // connection manager to use
        string wuType = default("org.mixim.base.modules.BaseWorldUtility");            // world utility to use

		@display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=0,0");

    submodules:
        connectionManager: <cmType> like IConnectionManager {
            parameters:
                @display("p=201,0;i=abstract/multicast;is=s");
        }
        world: <wuType> like IWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=280,0;i=misc/globe;is=s");
        }
    connections allowunconnected:
}

// network consists of master node, slave node, and relay node
network Network extends MyBaseNetwork
{
    parameters:
        int numHosts; // number of hosts in the network
        int numRelay; // number of relay in the network
        int numSlave; // number of slave in the network
        int numMaster; // number of master in the network
        cmType = default("org.mixim.base.connectionManager.ConnectionManager");	// connection manager to use
        wuType = default("org.mixim.examples.ieee802154Narrow.WorldUtilityStats"); // world untility to use
        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");

    submodules:
        mnode: MasterNode 
        // mnode: MasterCC2420
        {
         	numHosts = numMaster;
            @display("p=72,82;b=42,42,rect,yellow;i=device/wifilaptop");
        }

        rnode[numRelay]: RelayNode 
        {
            numHosts = numRelay;
            @display("p=210,82;b=42,42,rect,yellow;i=device/wifilaptop");
        }

        snode[numSlave]: SlaveNode
        // snode[numSlave]: SlaveCC2420
        {
            numHosts = numSlave;
            @display("p=146,82;b=42,42,rect,yellow;i=device/wifilaptop");
        }

    connections allowunconnected:	// all connections and gates are to be generated dynamically

}
