//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "InOutComing.h"
#include "CanDataFrame_m.h"
#include "CTFrame.h"
#include "TransportMessage_m.h"
#include "InterConnectMsg_m.h"
#include "MultipleFieldSequenceMessage.h"
#include "FieldElement.h"
#include "IdentifierFieldElement.h"
#include "Utility.h"
#include "cxmlelement.h"
#include "Ethernet.h"

Define_Module(InOutComing);

void InOutComing::initialize()
{

}

void InOutComing::handleMessage(cMessage *msg)
{

    if(msg->arrivedOn("appInterface$i",0) || msg->arrivedOn("appInterface$i",1)){
        TransportMessage *transportMsg = dynamic_cast<TransportMessage*>(msg);
        InterConnectMsg *interDataStructure = new InterConnectMsg;
        interDataStructure->setFirstArrivalTimeOnCan(transportMsg->getFirstArrivalTimeOnCan());

        cPacket *delivery = transportMsg->decapsulate();
        if(dynamic_cast<FiCo4OMNeT::CanDataFrame*>(delivery) != NULL){
            interDataStructure->encapsulate(delivery);
        }else if (dynamic_cast<CoRE4INET::CTFrame*>(delivery) != NULL){
            CoRE4INET::CTFrame *ctFrame = dynamic_cast<CoRE4INET::CTFrame*>(delivery);
            MultipleFieldSequenceMessage *multiFieldSequence = dynamic_cast<MultipleFieldSequenceMessage*>(ctFrame->decapsulate());
            interDataStructure->encapsulate(multiFieldSequence);
        }else if(dynamic_cast<EthernetIIFrame*>(delivery) != NULL){
            EthernetIIFrame *ethernetFrame = dynamic_cast<EthernetIIFrame*>(delivery);
            MultipleFieldSequenceMessage *multiFieldSequence = dynamic_cast<MultipleFieldSequenceMessage*>(ethernetFrame->decapsulate());
            interDataStructure->encapsulate(multiFieldSequence);
        }
        send(interDataStructure, "out");
    }else if(msg->arrivedOn("in")){
        InterConnectMsg *interDataStructure = dynamic_cast<InterConnectMsg*>(msg);
        TransportMessage *transportMsg = new TransportMessage;
        cPacket *delivery = interDataStructure->decapsulate();

        if(dynamic_cast<FiCo4OMNeT::CanDataFrame*>(delivery) != NULL){
            transportMsg->encapsulate(delivery);
            send(transportMsg, "appInterface$o", 0);
        }else if(dynamic_cast<MultipleFieldSequenceMessage*>(delivery) != NULL){

            transportMsg->setBackboneTransferType(interDataStructure->getBackboneTransferType());
            if(strcmp(interDataStructure->getBackboneTransferType(), "BG") == 0){
                EthernetIIFrame *bgFrame = new EthernetIIFrame();
                MACAddress address(interDataStructure->getDirectMacAdress());
                bgFrame->setDest(address);
                bgFrame->encapsulate(delivery);
                bgFrame->addByteLength(delivery->getByteLength());
                //Padding
                if (bgFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
                    bgFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
                }
                transportMsg->encapsulate(bgFrame);
            }else{
                CoRE4INET::CTFrame *ctFrame = new CoRE4INET::CTFrame("");
                EV << "CTID: " << interDataStructure->getBackboneCTID();
                ctFrame->setCtID(interDataStructure->getBackboneCTID());
                ctFrame->encapsulate(delivery);
                transportMsg->encapsulate(ctFrame);
            }

            send(transportMsg, "appInterface$o", 1);
        }
    }
    delete msg;
}
