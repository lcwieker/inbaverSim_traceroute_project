//
// Created by Finke, Tarek & Wieker, Lars as part of the Network Simulation Project
//

package inbaversim;

//
// Client application that sends traceroute requests
// DUMMY
//
simple TraceRouteApp like IApplication
{
    parameters:
        
        // time between content downloads
        volatile double startOffset @unit(s) = default(uniform(200s,500s));

        // requested prefixs
        string requestedPrefixNames = default("ccnx://universe.com/;ccnx://spacetime.com/");

        // file name prefix (a number is suffixed, based on nextFileSuffix)
        string dataNamePrefix = default("file");

        // maximum files stored at requested prefix
        volatile int nextFileSuffix = default(intuniform(0, 9999));

        // maximum hops allowed for at start
        int maxHopsAllowed = default(1);

        // time after which an interest is retransmitted
        double interestRetransmitTimeout @unit(s) = default(5s);
        
        // statistic signals
//        @signal[appContentDownloadDuration](type=simtime_t);
//        @signal[appSegmentDownloadDuration](type=simtime_t);
//        @signal[appTotalInterestsBytesSent](type=long);
//        @signal[appRetransmissionInterestsBytesSent](type=long);
//        @signal[appTotalContentObjsBytesReceived](type=long);
//        @signal[appTotalDataBytesReceived](type=long);
//        @signal[appNetworkInterestRetransmissionCount](type=long);
//        @signal[appNetworkInterestInjectedCount](type=long);
		  @signal[tracerouteRtt](type=simtime_t);
		  @signal[tracerouteRuntime](type=simtime_t);
		  
		  @statistic[tracerouteRtt](source=tracerouteRtt; record=stats,histogram,vector);
		  @statistic[tracerouteRuntime](source=tracerouteRuntime; record=stats,histogram,vector);

		// display icon
        @display("i=block/app");
        
    gates:
        inout forwarderInOut;
}
