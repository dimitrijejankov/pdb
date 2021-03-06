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
#ifndef PIPELINE_CONTEXT_H
#define PIPELINE_CONTEXT_H

// by Jia, Oct 2016

#include "Handle.h"
#include "PDBVector.h"
#include "Object.h"
#include "DataProxy.h"
#include "SetSpecifier.h"
#include <memory>


namespace pdb {

class PipelineContext;
typedef std::shared_ptr<PipelineContext> PipelineContextPtr;

// this class encapsulates the global state that is shared by pipeline nodes in the same pipeline
// network
class PipelineContext {

public:
    // temporarily make this public for getRecord()
    // the final output vector that needs to invoke getRecord() on
    Handle<Vector<Handle<Object>>> outputVec;


private:
    // the proxy to pin/unpin output page
    DataProxyPtr proxy;

    // the output set identifier
    SetSpecifierPtr outputSet;

    // whether output page is full
    bool outputPageFull;

    // page to unpin
    PDBPagePtr pageToUnpin;


public:
    ~PipelineContext() {
        this->outputVec = nullptr;
        this->outputSet = nullptr;
    }


    PipelineContext(Handle<Vector<Handle<Object>>> outputVec,
                    DataProxyPtr proxy,
                    SetSpecifierPtr outputSet) {
        this->outputVec = outputVec;
        this->proxy = proxy;
        this->outputSet = outputSet;
        this->outputPageFull = false;
        this->pageToUnpin = nullptr;
    }

    Handle<Vector<Handle<Object>>>& getOutputVec() {
        return this->outputVec;
    }

    void setOutputVec(Handle<Vector<Handle<Object>>> outputVec) {
        this->outputVec = outputVec;
    }

    void clearOutputPage() {
        outputVec = nullptr;
    }

    DataProxyPtr getProxy() {
        return this->proxy;
    }

    SetSpecifierPtr getOutputSet() {
        return this->outputSet;
    }

    void setOutputFull(bool fullOrNot) {
        this->outputPageFull = fullOrNot;
    }

    bool isOutputFull() {
        return this->outputPageFull;
    }

    void setPageToUnpin(PDBPagePtr page) {
        this->pageToUnpin = page;
    }

    PDBPagePtr getPageToUnpin() {
        return this->pageToUnpin;
    }
};
}


#endif
