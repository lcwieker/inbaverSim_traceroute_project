//  Created by Finke, Tarek & Wieker, Lars as part of the Network Simulation Project
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inbaversim.simulations;

import inbaversim.AccessRouter;
import inbaversim.ContentServer;
import inbaversim.CoreRouter;
import inbaversim.Demiurge;
import inbaversim.WiredNode;
import inbaversim.WirelessAccessRouter;
import inbaversim.WirelessNode;
import ned.DatarateChannel;

network TracerouteTestNetwork
{
    parameters:

        // statistics
        @statistic[appNetworkInterestRetransmissionCount](source=appNetworkInterestRetransmissionCount; record=last,vector);
        @statistic[appNetworkInterestInjectedCount](source=appNetworkInterestInjectedCount; record=last,vector);

        @statistic[fwdNetworkCacheHitRatio](source=fwdNetworkCacheHitRatio; record=min,max,count,last,vector);
        @statistic[fwdNetworkCacheMissRatio](source=fwdNetworkCacheMissRatio; record=min,max,count,last,vector);
        @statistic[fwdInterestToContentRatio](source=fwdInterestToContentRatio; record=min,max,count,last,vector,mean);
        @statistic[fwdRequestToReplyRatio](source=fwdRequestToReplyRatio; record=min,max,count,last,vector,mean);
        @statistic[fwdTotalTraffic](source=fwdTotalTraffic; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdTotalTrafficBytes](source=fwdTotalTrafficBytes; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdtotalExpiredPITCountSignal](source=fwdtotalExpiredPITCountSignal; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdtotalSatisfiedPITCountSignal](source=fwdtotalSatisfiedPITCountSignal; record=min,max,count,last,vector,sum,mean);
        @statistic[fwddurationOfPITEntrySignal](source=fwddurationOfPITEntrySignal; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdtotalCacheEntriesCountSignal](source=fwdtotalCacheEntriesCountSignal; record=min,max,count,last,vector,sum,mean);
        @statistic[fwddurationOfFirstCacheEmitSignal](source=fwddurationOfFirstCacheEmitSignal; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdTotalNetworkPITEntryCount](source=fwdTotalNetworkPITEntryCount; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdTotalNetworkFIBEntryCount](source=fwdTotalNetworkFIBEntryCount; record=min,max,count,last,vector,sum,mean);
        @statistic[fwdTotalNetworkCSEntryCount](source=fwdTotalNetworkCSEntryCount; record=min,max,count,last,vector,sum,mean);

        @display("bgb=400,400;bgg=100,0,grey95;bgu=m");

    submodules:
        accessRouter1: AccessRouter {
            @display("p=64,103");
        }
        accessRouter2: AccessRouter {
            @display("p=190,81");
        }
        accessRouter3: AccessRouter {
            @display("p=310,119");
        }

        wiredNode1: WiredNode {
            @display("p=6,63");
        }
        wiredNode2: WiredNode {
            @display("p=35,18");
        }
        wiredNode3: WiredNode {
            @display("p=90,57");
        }
        wiredNode4: WiredNode {
            @display("p=153,40");
        }
        wiredNode5: WiredNode {
            @display("p=190,1");
        }
        wiredNode6: WiredNode {
            @display("p=257,10");
        }
        wiredNode7: WiredNode {
            @display("p=291,63");
        }
        wiredNode8: WiredNode {
            @display("p=362,40");
        }
        wiredNode9: WiredNode {
            @display("p=362,104");
        }

        coreRouter1: CoreRouter {
            @display("p=53,165");
        }
        coreRouter2: CoreRouter {
            @display("p=110,219");
        }
        coreRouter3: CoreRouter {
            @display("p=43,269");
        }
        coreRouter4: CoreRouter {
            @display("p=190,176");
        }
        coreRouter5: CoreRouter {
            @display("p=310,176");
        }
        coreRouter6: CoreRouter {
            @display("p=347,293");
        }
        coreRouter7: CoreRouter {
            @display("p=130,126");
        }
        coreRouter8: CoreRouter {
            @display("p=237,137");
        }
        coreRouter9: CoreRouter {
            @display("p=220,323");
        }
        coreRouter10: CoreRouter {
            @display("p=275,269");
        }
        coreRouter11: CoreRouter {
            @display("p=110,284");
        }
        coreRouter12: CoreRouter {
            @display("p=130,361");
        }


        contentServer1: ContentServer {
            @display("p=30,346");
        }
        contentServer2: ContentServer {
            @display("p=200,231");
        }
        contentServer3: ContentServer {
            @display("p=362,361");
        }

        demiurge: Demiurge {

            @display("p=246,380");
        }
    connections allowunconnected:
        
        // endnodes
        wiredNode1.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter1.wiredInOut++;
        wiredNode2.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter1.wiredInOut++;
        wiredNode3.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter1.wiredInOut++;
        
        wiredNode4.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter2.wiredInOut++;
        wiredNode5.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter2.wiredInOut++;
        wiredNode6.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter2.wiredInOut++;
        
        wiredNode7.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter3.wiredInOut++;
        wiredNode8.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter3.wiredInOut++;
        wiredNode9.wiredInOut++ <--> DatarateChannel {  datarate = 1e6bps; } <--> accessRouter3.wiredInOut++;
        
        
        // acces routers
        accessRouter1.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter1.wiredInOut++;
        accessRouter2.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter4.wiredInOut++;
        accessRouter3.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter5.wiredInOut++;
        
        
        //core routers
        coreRouter1.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter2.wiredInOut++;
        coreRouter1.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter7.wiredInOut++;
        
        coreRouter2.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter3.wiredInOut++;
        coreRouter2.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter9.wiredInOut++;
        coreRouter2.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter11.wiredInOut++;
        
        coreRouter4.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter7.wiredInOut++;
        coreRouter4.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter8.wiredInOut++;
        
        coreRouter5.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter6.wiredInOut++;
        coreRouter5.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter8.wiredInOut++;
        coreRouter5.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter10.wiredInOut++;
        
        coreRouter9.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter10.wiredInOut++;
        coreRouter9.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter12.wiredInOut++;
        
        coreRouter10.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter6.wiredInOut++;
        
        coreRouter11.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter12.wiredInOut++;
        
        
        // content servers
        contentServer1.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter3.wiredInOut++;
        contentServer2.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter4.wiredInOut++;
        contentServer3.wiredInOut++ <--> DatarateChannel {	datarate = 1e6bps; } <--> coreRouter6.wiredInOut++;
        
}
        