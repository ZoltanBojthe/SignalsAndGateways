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
#include "candataframe_m.h"
#include "CTFrame.h"
#include "TransportMessage_m.h"
#include "InterConnectMsg_m.h"
#include "FieldSequenceMessage_m.h"
#include "FieldElement.h"
#include "IdentifierFieldElement.h"

Define_Module(InOutComing);

void InOutComing::initialize()
{
    // TODO - Generated method body
}

void InOutComing::handleMessage(cMessage *msg)
{

    if(msg->arrivedOn("appInterface$i",0) || msg->arrivedOn("appInterface$i",1)){
        EV << "InOutComing: arrivedOn 'appInterface' " << endl;
        TransportMessage *transportMsg = dynamic_cast<TransportMessage*>(msg);

        InterConnectMsg *interDataStructure = new InterConnectMsg;
        interDataStructure->setTransformationID("canTocan");
        cPacket *delivery = transportMsg->decapsulate();
        EV << "routing: " << "canTocan" << endl;
        EV << "typeid of delivery: " << typeid(delivery).name() << endl;
        if(dynamic_cast<CanDataFrame*>(delivery) != NULL){
            EV << "routing: " << "CanDataFrame" << endl;
            interDataStructure->setFrameFormat("canDataFrame");
            interDataStructure->encapsulate(delivery);
        }else if (dynamic_cast<EtherFrame*>(delivery) != NULL){
            EV << "routing: " << "EtherFrame" << endl;
            interDataStructure->setFrameFormat("transportFrame");
            CoRE4INET::CTFrame *ethernetFrame = dynamic_cast<CoRE4INET::CTFrame*>(delivery);
            FieldSequenceMessage *transportFrame = dynamic_cast<FieldSequenceMessage*>(ethernetFrame->decapsulate());
            interDataStructure->encapsulate(transportFrame);
        }
        send(interDataStructure, "out");
    }else if(msg->arrivedOn("in")){
        EV << "InOutComing: arrivedOn 'in' " << endl;
        InterConnectMsg *interDataStructure = dynamic_cast<InterConnectMsg*>(msg);
        TransportMessage *transportMsg = new TransportMessage;
        cPacket *delivery = interDataStructure->decapsulate();
        if(dynamic_cast<CanDataFrame*>(delivery) != NULL){
            EV << "InOutComing: sending out 'CanDataFrame' on 'appInterface - 0' " << endl;
            transportMsg->encapsulate(delivery);
            send(transportMsg, "appInterface$o", 0);
        }else if(dynamic_cast<FieldSequenceMessage*>(delivery) != NULL){
            EV << "InOutComing: sending out 'FieldSequenceMessage' on 'appInterface - 1' " << endl;
            CoRE4INET::CTFrame *ethernetFrame = new CoRE4INET::CTFrame("");
            FieldSequenceMessage *fieldSequence = dynamic_cast<FieldSequenceMessage*>(delivery);
            FieldSequenceDataStructure transportFrame = fieldSequence->getTransportFrame();

            EV << "transportFrame.size()" << transportFrame.size() << endl;
            std::shared_ptr<dataStruct::IdentifierFieldElement> specificElement  = transportFrame.getField<IdentifierFieldElement>();
            if(specificElement){
                uint16_t ctID = 0;
                EV << "InOutComing: identifierField : " << specificElement->getIdentifier() << endl;
                switch(specificElement->getIdentifier()){
                    case 1 : ctID = 100; break;
                    case 5 : ctID = 10; break;
                    case 2 : ctID = 150; break;
                    case 3 : ctID = 200; break;
                    default: ctID = USHRT_MAX;
                }
                ethernetFrame->setCtID(ctID);
                EV << "ethernetFrame->setCtID: " << ethernetFrame->getCtID() << endl;
            }else{
                opp_error("InOutComing: setting CTID on ethernetFrame failed!");
            }

            ethernetFrame->encapsulate(delivery);
            transportMsg->encapsulate(ethernetFrame);
            send(transportMsg, "appInterface$o", 1);
        }
    }
    //delete frame;
}
