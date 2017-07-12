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
#ifndef TPCH_GET_CUSTOMER_COUNT_CC
#define TPCH_GET_CUSTOMER_COUNT_CC

#include "CatalogClient.h"

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>

#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "QueryClient.h"
#include "DistributedStorageManagerClient.h"
#include "DispatcherClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"

#include "Part.h"
#include "Supplier.h"
#include "LineItem.h"
#include "Order.h"
#include "Customer.h"
#include "SumResultWriteSet.h"
#include "CustomerWriteSet.h"
#include "CustomerSupplierPartGroupBy.h"
#include "CustomerMultiSelection.h"
#include "ScanCustomerSet.h"
#include "SupplierData.h"
#include "CountAggregation.h"
#include "SumResult.h"


#include "Handle.h"
#include "Lambda.h"
#include "QueryClient.h"
#include "DistributedStorageManagerClient.h"
#include "DispatcherClient.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "SelectionComp.h"
#include "WriteBuiltinEmployeeSet.h"
#include "SupervisorMultiSelection.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"

#include "QueryOutput.h"
#include "DataTypes.h"

#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <fcntl.h>
#include "WriteStringSet.h"

using namespace std;

// Run a Cluster on Localhost
// ./bin/pdb-cluster localhost 8108 Y
// ./bin/pdb-server 4 4000 localhost:8108 localhost:8109
// ./bin/pdb-server 4 4000 localhost:8108 localhost:8110

// TPCH data set is available here https://drive.google.com/file/d/0BxMSXpJqaNfNMzV1b1dUTzVqc28/view
// Just unzip the file and put the folder in main directory of PDB

#define KB 1024
#define MB (1024*KB)
#define GB (1024*MB)

#define BLOCKSIZE (256*MB)



int main(int argc, char * argv[]) {


	// Connection info
	string masterHostname = "localhost";
	int masterPort = 8108;

	// register the shared employee class
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");

	pdb::DistributedStorageManagerClient distributedStorageManagerClient(masterPort, masterHostname, clientLogger);
	pdb::CatalogClient catalogClient(masterPort, masterHostname, clientLogger);
	pdb::DispatcherClient dispatcherClient = DispatcherClient(masterPort, masterHostname, clientLogger);
	pdb::QueryClient queryClient(masterPort, masterHostname, clientLogger, true);

	string errMsg;

	pdb::makeObjectAllocatorBlock((size_t) 2 * GB, true);



	// WE CHECK THE NUBMER OF STORED CUSTOMERS

	// now, create the sets for storing Customer Data
	if (!distributedStorageManagerClient.createSet<CustomerWriteSet>("TPCH_db", "output_setCustomer", errMsg)) {
		cout << "Not able to create set: " + errMsg;
		exit(-1);
	} else {
		cout << "Created set.\n";
	}
	// for allocations
	const UseTemporaryAllocationBlock tempBlock_Customers { 1024 * 1024 * 128 };

	// make the query graph
	Handle<Computation> myScanSet_CUSTOMER = makeObject<ScanCustomerSet>("TPCH_db", "tpch_bench_set1");

	Handle<Computation> myWriteSet_Customer = makeObject<CustomerWriteSet>("TPCH_db", "output_setCustomer");
	myWriteSet_Customer->setInput(myScanSet_CUSTOMER);

	auto begin = std::chrono::high_resolution_clock::now();

	if (!queryClient.executeComputations(errMsg, myWriteSet_Customer)) {
		std::cout << "Query failed. Message was: " << errMsg << "\n";
		return 1;
	}

	std::cout << std::endl;
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Time Duration: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << " ns." << std::endl;

	SetIterator<Customer> result_Customers = queryClient.getSetIterator<Customer>("TPCH_db", "output_setCustomer");
	int customerCount = 0;

	// a set to check if we stored all of the customers correctly.
	set<int> supplierIDs;

	// GO deep inside each customer and add their suppliers IDs to the set.
	for (auto a : result_Customers) {
		customerCount++;
	}

	std::cout << "Number of Customers Stored:" << customerCount << "\n";


	// CLEAN UP. Remove the Customer output set
	if (!distributedStorageManagerClient.removeSet("TPCH_db", "output_setCustomer", errMsg)) {
		cout << "Not able to remove the set: " + errMsg;
		exit(-1);
	} else {
		cout << "Set removed. \n";
	}

	// Clean up the SO files.
	int code = system("scripts/cleanupSoFiles.sh");
	if (code < 0) {

		std::cout << "Can't cleanup so files" << std::endl;

	}

}

#endif

