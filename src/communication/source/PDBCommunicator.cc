/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/


#ifndef PDB_COMMUN_C
#define PDB_COMMUN_C

#include "BuiltInObjectTypeIDs.h"
#include "Handle.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include "Object.h"
#include "PDBVector.h"
#include "CloseConnection.h"
#include "UseTemporaryAllocationBlock.h"
#include "InterfaceFunctions.h"
#include "PDBCommunicator.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>

namespace pdb {

PDBCommunicator::PDBCommunicator() {
    readCurMsgSize = false;
    socketFD = -1;
    nextTypeID = NoMsg_TYPEID;
    
    //Jia: moved this logic from Chris' message-based communication framework to here
    needToSendDisconnectMsg = false;
}

bool PDBCommunicator::pointToInternet(PDBLoggerPtr logToMeIn, int socketFDIn, std :: string& errMsg) {

    // first, connect to the backend
    logToMe = logToMeIn;

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof (cli_addr);
    logToMe->info("PDBCommunicator: about to wait for request from Internet");
    socketFD = accept(socketFDIn, (struct sockaddr *) &cli_addr, &clilen);
    if (socketFD < 0) {
        logToMe->error("PDBCommunicator: could not get FD to internet socket");
        logToMe->error(strerror(errno));
        errMsg = "Could not get socket ";
        errMsg += strerror(errno);
        return true;
    }

    logToMe->info("PDBCommunicator: got request from Internet");
    return false;
}

bool PDBCommunicator::connectToInternetServer(PDBLoggerPtr logToMeIn, int portNumber, std :: string serverAddress,
        std :: string &errMsg) {

    logToMe = logToMeIn;
    logToMe->trace("PDBCommunicator: creating socket....");

    // set up the socket
    struct sockaddr_in serv_addr;
    struct hostent *server;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        logToMe->error("PDBCommunicator: could not get FD to internet socket");
        logToMe->error(strerror(errno));
        errMsg = "Could not get socket to backend ";
        errMsg += strerror(errno);
        return true;
    }

    logToMe->trace("PDBCommunicator: Got internet socket");
    logToMe->trace("PDBCommunicator: About to check the database for the host name");

    /* CHRIS NOTE: turns out that gethostbyname () is depricated, and should be replaced */
    server = gethostbyname(serverAddress.c_str());
    if (server == nullptr) {
        logToMe->error("PDBCommunicator: could not get host by name");
        logToMe->error(strerror(errno));
        errMsg = "Could not get host by name ";
        errMsg += strerror(errno);
        return true;
    }

    logToMe->trace("PDBCommunicator: About to connect to the remote host");

    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portNumber);
    if (::connect(socketFD, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        logToMe->error("PDBCommunicator: could not get host by name");
        logToMe->error(strerror(errno));
        errMsg = "Could not connect to server ";
        errMsg += strerror(errno);
        return true;
    }

    // Jia: moved automatic tear-down logic from Chris' message-based communication to here
    // note that we need to close this up when we are done
    needToSendDisconnectMsg = true;

    logToMe->trace("PDBCommunicator: Successfully connected to the remote host");
    logToMe->trace("PDBCommunicator: Socket FD is " + std :: to_string (socketFD));
    return false;
}

void PDBCommunicator::setNeedsToDisconnect(bool option){
    needToSendDisconnectMsg = option;
}

bool PDBCommunicator::connectToLocalServer(PDBLoggerPtr logToMeIn, std :: string fName, std :: string& errMsg) {

    logToMe = logToMeIn;
    struct sockaddr_un server;
    socketFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketFD < 0) {
        logToMe->error("PDBCommunicator: could not get FD to local server socket");
        logToMe->error(strerror(errno));
        errMsg = "Could not get FD to local server socket ";
        errMsg += strerror(errno);
        return true;
    }

    std :: cout << "In here!!\n";

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, fName.c_str());
    if (::connect(socketFD, (struct sockaddr *) &server, sizeof (struct sockaddr_un)) < 0) {
        logToMe->error("PDBCommunicator: could not connect to local server socket");
        logToMe->error(strerror(errno));
        errMsg = "Could not connect to local server socket ";
        errMsg += strerror(errno);
        return true;
    }

    // Jia: moved automatic tear-down logic from Chris' message-based communication to here
    // note that we need to close this up when we are done
    needToSendDisconnectMsg = true;
    std :: cout << "Connected!!\n";

    return false;
}

bool PDBCommunicator::pointToFile(PDBLoggerPtr logToMeIn, int socketFDIn, std :: string& errMsg) {

    // connect to the backend
    logToMe = logToMeIn;

    logToMe->trace("PDBCommunicator: about to wait for request from same machine");
    socketFD = accept(socketFDIn, 0, 0);
    if (socketFD < 0) {
        logToMe->error("PDBCommunicator: could not get FD to local socket");
        logToMe->error(strerror(errno));
        errMsg = "Could not get socket ";
        errMsg += strerror(errno);
        return true;
    }

    logToMe->trace("PDBCommunicator: got request from same machine");

    return false;
}

PDBCommunicator::~PDBCommunicator() {

    // Jia: moved below logic from Chris' message-based communication to here.
    // tell the server that we are disconnecting (note that needToSendDisconnectMsg is
    // set to true only if we are a client and we want to close a connection to the server
    if (needToSendDisconnectMsg && socketFD > 0) {
        UseTemporaryAllocationBlock tempBlock (1024);
        Handle <CloseConnection> temp = makeObject <CloseConnection> ();
        logToMe->trace("PDBCommunicator: closing connection to the server");
	std :: string errMsg;
	if (!sendObject (temp, errMsg)) {
	    logToMe->trace("PDBCommunicator: could not send close connection message");
	}
    }

    if (socketFD >= 0) {
        close(socketFD);
    }

}

int16_t PDBCommunicator::getObjectTypeID () {

	// if we do not know the next type ID, then get it
        if (!readCurMsgSize) {
            getSizeOfNextObject ();
        }
	return nextTypeID;
}

size_t PDBCommunicator::getSizeOfNextObject () {

    // if we have previously gotten the size, just return it
    if (readCurMsgSize) {
	return msgSize;
    }

    // make sure we got enough bytes... if we did not, then error out
    if (read(socketFD, &nextTypeID, sizeof (int16_t)) <  (int) sizeof (int16_t)) {
        logToMe->error("PDBCommunicator: could not read next message type");
        logToMe->error(strerror(errno));
        nextTypeID = NoMsg_TYPEID;

    // OK, we did get enough bytes
    } else {
    	  logToMe->trace("PDBCommunicator: typeID of next object is " + std :: to_string (nextTypeID));
    }

    logToMe->trace("PDBCommunicator: getting the size of the next object:");

    // make sure we got enough bytes... if we did not, then error out
    int bytesRead;
    if ((bytesRead = read(socketFD, &msgSize, sizeof (size_t))) <  (int) sizeof (size_t)) {
        logToMe->error ("PDBCommunicator: could not read next message size" + std :: to_string (bytesRead));
        logToMe->error(strerror(errno));
        msgSize = 0;

    // OK, we did get enough bytes
    } else {
    	  logToMe->trace("PDBCommunicator: size of next object is " + std :: to_string (msgSize));
    }

    readCurMsgSize = true;
    return msgSize;
}

bool PDBCommunicator::doTheWrite(char *start, char *end) {

    // and do the write
    while (end != start) {

        // write some bytes
        ssize_t numBytes = write(socketFD, start, end - start);
        // make sure they went through
        if (numBytes <= 0) {
            logToMe->error("PDBCommunicator: error in socket write");
	    logToMe->trace("PDBCommunicator: tried to write " + std :: to_string (end - start) + " bytes.\n");
    	    logToMe->trace("PDBCommunicator: Socket FD is " + std :: to_string (socketFD));
            logToMe->error(strerror(errno));
            return true;
        } else {
	    logToMe->trace("PDBCommunicator: wrote " + std :: to_string (numBytes) + " and are " + std :: to_string (end - start - numBytes) + " to go!");
	}
        start += numBytes;
    }
    return false;
}

bool PDBCommunicator :: doTheRead(char *dataIn) {

    if (!readCurMsgSize) {
        getSizeOfNextObject ();
    }
    readCurMsgSize = false;

    // now, read the rest of the bytes
    char *start = dataIn;
    char *cur = start;

    while (cur - start < (long) msgSize) {

        ssize_t numBytes = read(socketFD, cur, msgSize - (cur - start));
        this->logToMe->trace("PDBCommunicator: received bytes: " + std :: to_string (numBytes));
        this->logToMe->trace("PDBCommunicator: " + std :: to_string (msgSize - (cur - start)) + " bytes to go!");

        if (numBytes <= 0) {
            logToMe->error("PDBCommunicator: error reading socket when trying to accept text message");
            logToMe->error(strerror(errno));
            return true;
        }
        cur += numBytes;
    }
    return false;
}

}

#endif
