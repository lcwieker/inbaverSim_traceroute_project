//
// Created by Finke, Tarek & Wieker, Lars as part of the Network Simulation Project
//
#include "TraceRouteApp.h"

Define_Module(TraceRouteApp);

void TraceRouteApp::initialize(int stage)
{
    if (stage == 0) {

        // get parameters
        requestedPrefixNames = par("requestedPrefixNames").stringValue();
        dataNamePrefix = par("dataNamePrefix").stringValue();
        maxHopsAllowed = par("maxHopsAllowed");
        interestRetransmitTimeout = par("interestRetransmitTimeout");
        startOffset = par("startOffset");
        //ftraceSendTime = par("traceSendTime");


    } else if (stage == 1) {

        // get Demiurge model instance
        demiurgeModel = NULL;
        for (int id = 0; id <= getSimulation()->getLastComponentId(); id++) {
            cModule *unknownModel = getSimulation()->getModule(id);
            if (unknownModel == NULL) {
                continue;
            }
            if (dynamic_cast<Demiurge*>(unknownModel) != NULL) {
                demiurgeModel = dynamic_cast<Demiurge*>(unknownModel);
                break;
            }
        }

        // get Numen model instance
        numenModel = NULL;
        for (cModule::SubmoduleIterator it(getParentModule()); !it.end(); it++) {
            cModule *unknownModel = *it;
            if (unknownModel == NULL) {
                continue;
            }
            if (dynamic_cast<Numen*>(unknownModel) != NULL) {
                numenModel = dynamic_cast<Numen*>(unknownModel);
                break;
            }
        }

        // when Demiurge or Numen not found, terminate
        if (demiurgeModel == NULL || numenModel == NULL) {
            EV_FATAL << "The global Demiurge instance and/or node specific Numen instance not found.\n";
            throw cRuntimeError("Check log for details");
        }


        // make list of requested prefixes
        string ccnPrefix;
        stringstream stream(requestedPrefixNames);
        while(getline(stream, ccnPrefix, ';')) {
            requestedPrefixList.push_back(ccnPrefix);
        }
    } else if (stage == 2){

        // reminder to generate content host app registration event
        cMessage *appRegReminderEvent = new cMessage("App Registration Reminder Event");
        appRegReminderEvent->setKind(TRACEROUTEAPP_APP_REG_REM_EVENT_CODE);
        scheduleAt(simTime(), appRegReminderEvent);

        // start traceroute
        traceRouteStartEvent = new cMessage("Traceroute Start Event");
        traceRouteStartEvent->setKind(TRACEROUTEAPP_START_TRACEROUTE_EVENT_CODE);
        scheduleAt(simTime() + startOffset, traceRouteStartEvent);

        // Trace timeout event
        traceTimeoutEvent = new cMessage("Trace timeout");
        traceTimeoutEvent->setKind(TRACEROUTEAPP_TIMEOUT_EVENT_CODE);
        scheduleAt(simTime() + startOffset + interestRetransmitTimeout, traceTimeoutEvent);

        // register stat signals
        totalInterestsBytesSentSignal = registerSignal("appTotalInterestsBytesSent");
        retransmissionInterestsBytesSentSignal = registerSignal("appRetransmissionInterestsBytesSent");
        totalContentObjsBytesReceivedSignal = registerSignal("appTotalContentObjsBytesReceived");
        totalDataBytesReceivedSignal = registerSignal("appTotalDataBytesReceived");
        networkInterestRetransmissionCountSignal = registerSignal("appNetworkInterestRetransmissionCount");
        networkInterestInjectedCountSignal = registerSignal("appNetworkInterestInjectedCount");
        tracerouteRttSignal = registerSignal("tracerouteRtt");
    }
    else {
        EV_FATAL << "Something is radically wrong\n";
        throw cRuntimeError("Check log for details");
    }
}

void TraceRouteApp::handleMessage(cMessage *msg)
{
    TracerouteRplMsg *tracerouteRplMsg = NULL;

    // register app with lower layer (forwarder)
    if (msg->isSelfMessage() && msg->getKind() == TRACEROUTEAPP_APP_REG_REM_EVENT_CODE) {

        EV_INFO << simTime() << " Registering application with ID: " << getId() << endl;

        AppRegistrationMsg *appRegMsg = new AppRegistrationMsg();
        appRegMsg->setAppID(getId());
        appRegMsg->setAppDescription("Traceroute Client Application");

        send(appRegMsg, "forwarderInOut$o");

        delete msg;
    }

    else if (msg->isSelfMessage() && msg->getKind() == TRACEROUTEAPP_START_TRACEROUTE_EVENT_CODE){

            int rndNum;
            char tempString[128];
            // identify data to download
            requestingPrefixName = requestedPrefixList[0];
            rndNum = par("nextFileSuffix");
            snprintf(tempString, sizeof(tempString), "%s%04d", dataNamePrefix.c_str(), rndNum);
            requestingDataName = string(tempString);
            requestedSegNum = 0;
            totalSegments = -1;
            TraceStartTime = simTime();

            EV_INFO << simTime() << " New Trace for " << requestingPrefixName << " "
                    << requestingDataName << " v01" << " starts " << endl;

            // generate 1st interest
            TracerouteRqstMsg* tracerouteRqstMsg = new TracerouteRqstMsg("Interest");
            tracerouteRqstMsg->setHopLimit(maxHopsAllowed);
            tracerouteRqstMsg->setLifetime(simTime() + interestRetransmitTimeout);
            tracerouteRqstMsg->setPrefixName(requestingPrefixName.c_str());
            tracerouteRqstMsg->setDataName(requestingDataName.c_str());
            tracerouteRqstMsg->setVersionName("v01");
            tracerouteRqstMsg->setSegmentNum(requestedSegNum);
            tracerouteRqstMsg->setHeaderSize(INBAVER_INTEREST_MSG_HEADER_SIZE);
            tracerouteRqstMsg->setPayloadSize(0);
            tracerouteRqstMsg->setHopsTravelled(0);
            tracerouteRqstMsg->setByteLength(INBAVER_INTEREST_MSG_HEADER_SIZE);
            tracerouteRqstMsg->setRequestStartTime(simTime());
            tracerouteRqstMsg->setTracerouteToken(intuniform(100001, 1000000));

            EV_INFO << simTime() << " Sending Trace for: " << requestingPrefixName
                    << " " << requestingDataName << " v01 " << requestedSegNum
                    << " " << totalSegments << endl;

            // send msg to forwarding layer
            send(tracerouteRqstMsg, "forwarderInOut$o");

            maxHopsAllowed++;

            // remember last interest sent time for statistic
            lastTraceSentTime = simTime();
            //traceSendTime[];

            // update stats
            demiurgeModel->incrementNetworkInterestInjectedCount(); //@Lars sollten wir hier nicht unterscheiden -> incrementNetworkTracerouteRqstInjectedCount ? Ja der meinung bin ich auch, ich habe mir noch keine gedanken über die Statistik gemacht.

            // write stats
            //emit(totalInterestsBytesSentSignal, (long) tracerouteRqstMsg->getByteLength());
            //emit(networkInterestInjectedCountSignal, demiurgeModel->getNetworkInterestInjectedCount());

            //scheduleAt(simTime() + interestRetransmitTimeout, traceTimeoutEvent);



        }

        else if ((tracerouteRplMsg = dynamic_cast<TracerouteRplMsg*>(msg)) != NULL){

            bool finalReply = false;

            int rndNum;
            char tempString[128];
            const char* pathTLV;
            pathTLV = tracerouteRplMsg->getPathlabel();

            // identify data to download
            requestingPrefixName = requestedPrefixList[0];
            rndNum = par("nextFileSuffix");
            snprintf(tempString, sizeof(tempString), "%s%04d", dataNamePrefix.c_str(), rndNum);
            requestingDataName = string(tempString);
            requestedSegNum = 0;
            totalSegments = -1;
            //TraceStartTime = simTime();

            // catch reply code
            int replyCode = tracerouteRplMsg->getTracerouteReplyCode();

            switch(replyCode){

            case 4: //hopLImit
            {
                EV_INFO << simTime() << " Received Traceroute Reply for exceed hopLimit:"
                        << " " << tracerouteRplMsg->getPrefixName()
                        << " " << tracerouteRplMsg->getDataName()
                        << " " << tracerouteRplMsg->getVersionName()
                        << " " << tracerouteRplMsg->getSegmentNum()
                        << " /replyCode: " << tracerouteRplMsg->getTracerouteReplyCode()
                        << " /hopsTravelled: " << tracerouteRplMsg->getHopsTravelled()
                        << " at: " << simTime()
                        << " after: " << simTime() - tracerouteRplMsg->getRequestStartTime()
                        << " from: " << tracerouteRplMsg->getPayloadAsString()
                        << endl;

                // generate new traceroute request
                TracerouteRqstMsg* tracerouteRqstMsg = new TracerouteRqstMsg("Traceroute Request");
                tracerouteRqstMsg->setHopLimit(tracerouteRplMsg->getHopsTravelled() + 1);
                tracerouteRqstMsg->setLifetime(simTime() + interestRetransmitTimeout);
                tracerouteRqstMsg->setPrefixName(tracerouteRplMsg->getPrefixName());
                tracerouteRqstMsg->setDataName(tracerouteRplMsg->getDataName()); //@Lars Traceroute should repeat searching for the same data
                tracerouteRqstMsg->setVersionName("v01");
                tracerouteRqstMsg->setSegmentNum(requestedSegNum);
                tracerouteRqstMsg->setHeaderSize(INBAVER_INTEREST_MSG_HEADER_SIZE);
                tracerouteRqstMsg->setPayloadSize(0);
                tracerouteRqstMsg->setHopsTravelled(0);
                tracerouteRqstMsg->setByteLength(INBAVER_INTEREST_MSG_HEADER_SIZE);
                tracerouteRqstMsg->setPathlabel(pathTLV);
                tracerouteRqstMsg->setRequestStartTime(simTime());
                tracerouteRqstMsg->setTracerouteToken(tracerouteRplMsg->getTracerouteToken());

                EV_INFO << simTime() << " Sending next Trace for: " << requestingPrefixName
                        << " " << requestingDataName << " v01 " << requestedSegNum
                        << " " << totalSegments << endl;

                // for timeout event
                maxHopsAllowed = tracerouteRqstMsg->getHopLimit();

                // send msg to forwarding layer
                sendDelayed(tracerouteRqstMsg, 1, "forwarderInOut$o");

                // remember last interest sent time for statistic
                emit(tracerouteRttSignal, (simtime_t) simTime() - tracerouteRplMsg->getRequestStartTime());
                //lastTraceSentTime = simTime();

                // update stats
                demiurgeModel->incrementNetworkInterestInjectedCount();

                // write stats
                //emit(totalInterestsBytesSentSignal, (long) tracerouteRqstMsg->getByteLength());
                //emit(networkInterestInjectedCountSignal, demiurgeModel->getNetworkInterestInjectedCount());

                dataNodes.push_back({tracerouteRplMsg->getPathlabel(), tracerouteRplMsg->getPayloadAsString(), (simtime_t)(simTime() - tracerouteRplMsg->getRequestStartTime()),
                                        false});

                if (traceTimeoutEvent -> isScheduled()){

                    cancelEvent(traceTimeoutEvent);

                    scheduleAt(simTime() + interestRetransmitTimeout, traceTimeoutEvent);

                }

                EV_INFO << "Scanned nodes so far:\n"
                        << "Name | Route | RTT"
                        << endl;

                for(const auto& dataNode : dataNodes){
                    EV_INFO << dataNode.name << " | " << dataNode.pathlabel << " | " << dataNode.RTT
                            << endl;
                }

                delete tracerouteRplMsg;
                break;
            }

            case 3: //noPath
            {
                EV_INFO << simTime() << " Received Traceroute Reply for dead end:"
                        << " " << tracerouteRplMsg->getPrefixName()
                        << " " << tracerouteRplMsg->getDataName()
                        << " " << tracerouteRplMsg->getVersionName()
                        << " " << tracerouteRplMsg->getSegmentNum()
                        << " /replyCode: " << tracerouteRplMsg->getTracerouteReplyCode()
                        << " /hopsTravelled: " << tracerouteRplMsg->getHopsTravelled()
                        << " at: " << simTime()
                        << " after: " << simTime() - tracerouteRplMsg->getRequestStartTime()
                        << " from: " << tracerouteRplMsg->getPayloadAsString()
                        << endl;

                // remember last interest sent time for statistic
                emit(tracerouteRttSignal, (simtime_t) simTime() - tracerouteRplMsg->getRequestStartTime());


                //enlist replying node if not duplicate
                auto isDuplicate = [&](const DataNode& dn){
                    return any_of(dataNodes.begin(), dataNodes.end(), [&](const DataNode& existing){
                        return dn.pathlabel == existing.pathlabel && dn.name == existing.name;
                    });
                };

                DataNode newNode{tracerouteRplMsg->getPathlabel(), tracerouteRplMsg->getPayloadAsString(), (simtime_t)(simTime() - tracerouteRplMsg->getRequestStartTime()),
                                    false};

                if(!isDuplicate(newNode)){
                    dataNodes.push_back(newNode);
                }



                EV_INFO << "Scanned nodes so far:\n"
                        << "Name | Route | RTT"
                        << endl;

                for(const auto& dataNode : dataNodes){
                    EV_INFO << dataNode.name << " | " << dataNode.pathlabel << " | " << dataNode.RTT
                            << endl;
                }

                delete tracerouteRplMsg;
                break;
            }

            default: //found in full forwarder match, CS hit or local name
            {
                EV_INFO << simTime() << " Received final Traceroute Reply:"
                        << " " << tracerouteRplMsg->getPrefixName()
                        << " " << tracerouteRplMsg->getDataName()
                        << " " << tracerouteRplMsg->getVersionName()
                        << " " << tracerouteRplMsg->getSegmentNum()
                        << " /replyCode: " << tracerouteRplMsg->getTracerouteReplyCode()
                        << " /hopsTravelled: " << tracerouteRplMsg->getHopsTravelled()
                        << " at: " << simTime()
                        << " after: " << simTime() - tracerouteRplMsg->getRequestStartTime()
                        << " from: " << tracerouteRplMsg->getPayloadAsString()
                        << endl;

                // remember last interest sent time for statistic
                emit(tracerouteRttSignal, (simtime_t) simTime() - tracerouteRplMsg->getRequestStartTime());


                //enlist replying node if not duplicate
                auto isDuplicate = [&](const DataNode& dn){
                    return any_of(dataNodes.begin(), dataNodes.end(), [&](const DataNode& existing){
                        return dn.pathlabel == existing.pathlabel && dn.name == existing.name;
                    });
                };

                DataNode newNode{tracerouteRplMsg->getPathlabel(), tracerouteRplMsg->getPayloadAsString(), (simtime_t)(simTime() - tracerouteRplMsg->getRequestStartTime()),
                                    true};

                if(!isDuplicate(newNode)){
                    dataNodes.push_back(newNode);
                }


                EV_INFO << "Scanned nodes so far:\n"
                        << "Name | Route | RTT"
                        << endl;

                for(const auto& dataNode : dataNodes){
                    EV_INFO << dataNode.name << " | " << dataNode.pathlabel << " | " << dataNode.RTT
                            << endl;
                }


                finalReply = true;
                delete tracerouteRplMsg;
                break;
            }
            }

            if(finalReply){
                emit(tracerouteRuntime, (simtime_t) simTime() - TraceStartTime);
                //TODO Auswertung
                /* How to:
                 * 1. In dataNodes, find latest entry with final = true and print name and RTT
                 * 2. Take pathlabel of said entry, delete the last char for new search
                 * 3. Find entry with matching pathlabel, print name and RTT
                 * 4. repeat 2-4 until pathlabel empty
                 */


                EV << "Found Data Source:\n"
                        << "Name | RTT"
                        << endl;

                string tempLabel = "";
                for(auto& dataNode : dataNodes){
                    if(dataNode.final){
                        EV << dataNode.name << " | " << dataNode.RTT << endl;
                        tempLabel = dataNode.pathlabel;
                        dataNode.final = false;
                        break;
                    }
                }

                while(tempLabel.size() > 0){
                    tempLabel.pop_back();
                    for(const auto& dataNode : dataNodes){
                        if(dataNode.pathlabel == tempLabel){
                            EV << dataNode.name << " | " << dataNode.RTT << endl;
                            break;
                        }
                    }
                }
            }

        }

        else if (msg->isSelfMessage() && msg->getKind() == TRACEROUTEAPP_TIMEOUT_EVENT_CODE){

                       EV << "Trace timeout. Trace has finished.";
                       maxHopsAllowed = par("maxHopsAllowed");
                       pathTLV = "";

                       finish();

        }
}

void TraceRouteApp::finish(){
    // remove remaining events
    if (traceRouteStartEvent -> isScheduled()){
        cancelEvent(traceRouteStartEvent);
    }
    delete traceRouteStartEvent;

    if (traceTimeoutEvent -> isScheduled()){
        cancelEvent(traceTimeoutEvent);
    }
    delete traceTimeoutEvent;
}
