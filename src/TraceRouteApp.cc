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
            TraceStartTime = simTime();

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
                        << endl;

                // generate new traceroute request
                TracerouteRqstMsg* tracerouteRqstMsg = new TracerouteRqstMsg("Traceroute Request");
                tracerouteRqstMsg->setHopLimit(maxHopsAllowed);
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

                EV_INFO << simTime() << " Sending next Trace for: " << requestingPrefixName
                        << " " << requestingDataName << " v01 " << requestedSegNum
                        << " " << totalSegments << endl;

                // send msg to forwarding layer
                sendDelayed(tracerouteRqstMsg, uniform(1, 5), "forwarderInOut$o");

                // remember last interest sent time for statistic
                lastTraceSentTime = simTime();

                // update stats
                demiurgeModel->incrementNetworkInterestInjectedCount();

                // write stats
                emit(totalInterestsBytesSentSignal, (long) tracerouteRqstMsg->getByteLength());
                emit(networkInterestInjectedCountSignal, demiurgeModel->getNetworkInterestInjectedCount());

                maxHopsAllowed++;

                if (traceTimeoutEvent -> isScheduled()){

                    cancelEvent(traceTimeoutEvent);

                    scheduleAt(simTime() + interestRetransmitTimeout, traceTimeoutEvent);

                }
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
                        << endl;

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
                        << endl;

                finalReply = true;
                delete tracerouteRplMsg;
                break;
            }
            }

            if(finalReply){
                //TODO Auswertung
            }

        }

        else if (msg->isSelfMessage() && msg->getKind() == TRACEROUTEAPP_TIMEOUT_EVENT_CODE){

                       EV << "Trace timeout. Trace has finished.";
                       maxHopsAllowed = par("maxHopsAllowed");
                       pathTLV = "";

        }
}

void TraceRouteApp::finish(){
    // remove remaining events
    if (traceRouteStartEvent -> isScheduled()){
        cancelEvent(traceRouteStartEvent);
    }
    delete traceRouteStartEvent;

    if (interestRetransmitEvent -> isScheduled()){
        cancelEvent(interestRetransmitEvent);
    }
    delete interestRetransmitEvent;
}
