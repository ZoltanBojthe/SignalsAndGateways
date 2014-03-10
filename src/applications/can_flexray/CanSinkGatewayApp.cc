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

#include "CanSinkGatewayApp.h"
#include "TransportMessage_m.h"
#include "candataframe_m.h"

Define_Module(CanSinkGatewayApp);

void CanSinkGatewayApp::handleMessage(cMessage *msg)
{
    std::string msgClass = msg->getClassName();
    EV << "Message getClassName() " << msgClass;
    if (msg->arrivedOn("controllerIn")) {
        bufferMessageCounter++;
        if (idle) {
            requestFrame();
        }
        delete msg;
    } else if (msgClass.compare("CanDataFrame") == 0) {
        CanDataFrame *frame = check_and_cast<CanDataFrame *>(msg);
        int i = frame->getCanID();
        currentFrameID = i;
        bufferMessageCounter--;
        TransportMessage *transFrame = new TransportMessage();
        transFrame->encapsulate(frame);
        send(transFrame, "busInterfaceOut");
    } else if (msg->isSelfMessage()) {
        InputBuffer *buffer = (InputBuffer*) (getParentModule()->getSubmodule(
                "bufferIn"));
        buffer->deleteFrame(currentFrameID);
        if (bufferMessageCounter > 0) {
            requestFrame();
        } else {
            idle = true;
        }
        delete msg;
    }
    //delete msg;
}