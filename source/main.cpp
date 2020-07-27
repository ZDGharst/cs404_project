#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <time.h>
#include <string>
#include <sstream>


#include "EmergencyVehicle.h"
#include "Request.h"
#include "Distance.h"
#include "debug.cpp"

/* Function proto-types. */
void read_zipcodes_csv(int[], Distance[]);
Request generate_random_request(int[]);

/* Global variables representing primary keys (starting at 1) for vehicles and requests ONLY. */
int request_primary_key = 1;
int vehicle_primary_key = 1;

int main() {
	/* Load zipcodes into an array (representing vertices of a graph).
	Load connections between zipcodes into an array (representing edges of a graph). */
	int zipcodes[24];
	Distance connections[39];
	read_zipcodes_csv(zipcodes, connections);

	/* Generate 10 of each vehicle type in a random zipcode of our dispatch area. */
	std::vector<EmergencyVehicle> vehicles;
	for (int i = 0; i < 10; i++) {
		int random_zipcode = rand() % 24;
		vehicles.push_back(EmergencyVehicle(vehicle_primary_key++, VehicleType::ambulance, zipcodes[random_zipcode]));
	}
	for (int i = 0; i < 10; i++) {
		int random_zipcode = rand() % 24;
		vehicles.push_back(EmergencyVehicle(vehicle_primary_key++, VehicleType::fire_truck, zipcodes[random_zipcode]));
	}
	for (int i = 0; i < 10; i++) {
		int random_zipcode = rand() % 24;
		vehicles.push_back(EmergencyVehicle(vehicle_primary_key++, VehicleType::police, zipcodes[random_zipcode]));
	}

	/* Declare data structures for unfulfilled requests. */
	std::queue<Request> unfulfilled_requests;

	/* Our control structure emulating time. Represents 24 hours; each iteration represents 1 minute. We will simulate a
	full day (24 * 60 = 1440). */
	srand(time(NULL));
	for (int i = 0; i < 1440; i++) {
		/* Every 10 minutes, generate a new request. */
		if (i % 10 == 0)
			unfulfilled_requests.push(generate_random_request(zipcodes));

		/* If there are requests awaiting a vehicle, look for the closest vehicle that matches the type of
		request that is available. If no vehicles of that type are available, wait for one to become available. */
		if (!unfulfilled_requests.empty()) {
			if (unfulfilled_requests.front().assign_vehicle(vehicles)) {
				if (debug::detailed_view) std::cout << i << "Vehicle #" << unfulfilled_requests.front().get_vehicle_id() << " assigned to help request #" << unfulfilled_requests.front().get_id() << '\n';
				unfulfilled_requests.pop();
			}
			else if (debug::detailed_view) std::cout << i << "Attempted to fulfill request #" << unfulfilled_requests.front().get_id() << " but there are no vehicles of that type available!\n";
		}

		/* Time has incremented by 1, so all busy vehicles must be updated to see if they are now available. */
		for (int i = 0; i < 30; i++) {
			vehicles[i].update();
		}
	}
	
	return 0;
}

Request generate_random_request(int locations[]) {
	/* Generate a random type from our enums; ambulance, fire, or police. */
	VehicleType random_type = static_cast<VehicleType>(rand() % 3 + 1);

	/* Generate a random zip code. */
	int random_zipcode = rand() % 24;

	/* Create the request and return it. Set the primary key of the request and increment it by one for
	the next request. */
	Request random_request(request_primary_key++, random_type, locations[random_zipcode]);

	if (debug::detailed_view) {
		std::cout << "Just got a call: person #" << random_request.get_id() << " at " << random_request.get_location() << " needs ";
		if (random_request.get_type() == VehicleType::ambulance) std::cout << "an ambulance";
		else if (random_request.get_type() == VehicleType::fire_truck) std::cout << "a fire truck";
		else if (random_request.get_type() == VehicleType::police) std::cout << "a police car";
		else std::cout << "null_type";
		std::cout << '\n';
	}

	return random_request;
}

void read_zipcodes_csv(int zipcodes[], Distance connections[]) {
	/* Zipcodes csv is formatted as follows:
	Line 1 is a list of the 24 zipcodes being used. 
	Line 2 and beyond are formatted as zipcode 1, zipcode 2, and distance between them, separated by commas. */
	std::ifstream zipcode_csv("zipcodes.csv");
	if (!zipcode_csv) throw std::runtime_error("Could not open file");

	/* Handle the first line which are the 24 zipcodes used. */
	std::string line;
	std::getline(zipcode_csv, line);
	std::stringstream ss(line);

	for (int i = 0; i < 24; i++) {
		ss >> zipcodes[i];
		ss.ignore();
	}

	/* Handle all remaining lines treating them as edges in a graph. */
	for (int i = 0; i < 39; i++) {
		std::getline(zipcode_csv, line);
		std::stringstream ss(line);

		ss >> connections[i].zipcode1;
		ss.ignore();
		ss >> connections[i].zipcode2;
		ss.ignore();
		ss >> connections[i].distance;
	}
}