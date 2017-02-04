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


package ts2;

// import org.mixim.base.modules.BaseNetwork;
import org.mixim.base.connectionManager.IConnectionManager;
import org.mixim.base.modules.IWorldUtility;
import org.mixim.examples.ieee802154Narrow.WorldUtilityStats;


network tsieee802154masterrelay extends MyBaseNetwork
{
    parameters:
        int numHosts = default(0); // number of hosts in the network, this parameter is useless in the network
        int numRelay; // number of relay in the network
		int numMaster; // number of master in the network
		cmType = default("org.mixim.base.connectionManager.ConnectionManager");	// connection manager to use
        wuType = default("org.mixim.examples.ieee802154Narrow.WorldUtilityStats"); // world untility to use
        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
        
    submodules:
        mnode: MasterNode {
            parameters:
                numHosts = numMaster;
                @display("p=147,100;b=42,42,rect,yellow;i=device/wifilaptop");

        }
        relay[numRelay]: RelayNode {
            parameters:
                numHosts = numRelay;
                @display("p=139,82;b=42,42,rect,yellow;i=device/wifilaptop");
        }

    connections allowunconnected:	// all connections and gates are to be generated dynamically

}
