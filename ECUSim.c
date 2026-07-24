#include <stdio.h>
#include <stdbool.h>
#include"tables.h"

#define loopSize 100   // Define how many finite steps are available for one ECU loop

// Tuning values
#define TPSCheck 100                // Rate at which the scheduler runs the TPS check
#define TPSDeadband 5               // Band at which toe-in enrichment does not occur
#define toeInEnrichmentDecay 95     // % of enrichment to keep per itteration

#define onBootAFR 12.5              // AFR Value to initialize with
#define toeEnrichment 1.5           // Toe-in enrichment multiplier
#define coldStartEnrichment 1.3     // Engine cold start enrichment
#define crankingEnrichment 1.2      // Engine cranking enrichment
#define initFuelTrim 1.0            // Manual fuel trim multiplier

const word16 DISPLACEMENT = 4;                            // Engine displacement in L
const word16 DISPLACEMENT_PER_REV = DISPLACEMENT / 2;     // This will be pre-calculated and stored in ROM

//  Utility functions
word16 KtoFConversion(int F){           // This is used to convert F for the user to the internal representation in Kelvin.
    return ((F-32) * 5 / 9) + 273.15;
}

struct Engine* get_engine_state() {     // To allow python to get the engine state
    return &engineInstance;
}

void calculateToeEnrichment(struct Engine *eng){
    int16_t deltaTPS = eng->TPS - eng->lastTPSValue;

    if(deltaTPS > TPSDeadband){
        eng->toeEnrichmentMultiplier = eng->TPS * toeEnrichment;
    }

    if (eng->toeEnrichmentMultiplier > 0){
        eng->toeEnrichmentMultiplier = (eng->toeEnrichmentMultiplier * toeInEnrichmentDecay) / 100;
    }

    if (eng->toeEnrichmentMultiplier < 2){
        eng->toeEnrichmentMultiplier = 1;
    }

    eng->lastTPSValue = eng->TPS;
}

void calculateAFR(struct Engine *eng){      // Fetches current AFR with lookup table
    int MAPBin = MAP_BINS - 1;
    int RPMBin = RPM_BINS - 1;

    word16 MAPKPA = eng->MAP / 100;#include <sys/types.h>
#include <stdint.h>

typedef uint16_t word16;
typedef uint32_t word32;

#define MAP_BINS 10
#define RPM_BINS 10

struct Engine {
    // Static engine variables
    int displacementPerRev; // Define the engine displacement in L
    int coldCoolant;        // Threshold under which coolant is considered cold

    // Dynamic engine variables
    // Sensors
    word16 TPS;     // Throttle position sensor
    word16 RPM;     // Engine speed in RPM
    word16 MAP;     // Manifold air pressure represented as Pa /10
    word16 AAP;     // Ambient pressure represented as ambient Pa /10
    word16 IAT;     // Intake air temperature measured in K
    word16 OX;      // Oxygen sensor voltage represented at V * 100
    word16 COOLANT; // Coolant tempurature in F
    word16 fuelTrim;// Manual fuel trim

    // Calculated values
    word16 fuelLoad;
    word16 VE;
    float AFR_TARGET;
    float fuelLoadCorrected;
    float toeEnrichmentMultiplier;

    // Engine Flags
    bool EngineCranking;    // Cranking state of engine
    bool Coldstart;         // Is the ECU Booting into a cold start condition

    // Utility variables
    word16 lastTPSValue;    // TPS Value on the last loop
    word16 TIFE;            // toe-in fuel enrichment

};

struct ECUSchedule {        // Schedule certian unimportant heavy tasks to leave headroom for primary calculations
    int ECULoopSize;        // Size of steps per loop of the scheduler
    int ECUStep;            // Current step of the loop the ECU is on
    
    int crankCheckInterval; // How often we check for a cranking condition
    int TPSCheckInterval;   // How often we check the throttle position sensor
};

// Y axis
const uint16_t mapAxis[MAP_BINS] = {15, 20, 30, 40, 50, 60, 70, 80, 90, 100};

// X axis
const uint16_t rpmAxis[RPM_BINS] = {600, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 5000, 6000};

// VE Table
const uint8_t VETable[MAP_BINS * RPM_BINS] = {
    // 600, 1k, 1.5k, 2k, 2.5k, 3k, 3.5k, 4k, 5k, 6k
    30, 33, 37, 40, 43, 45, 47, 45, 42, 40,  // 15 kPa
    33, 36, 40, 44, 47, 50, 52, 50, 47, 45,  // 20 kPa
    38, 42, 47, 52, 55, 58, 60, 58, 54, 51,  // 30 kPa
    43, 47, 53, 58, 62, 65, 67, 65, 60, 56,  // 40 kPa
    48, 52, 58, 64, 68, 71, 73, 71, 65, 61,  // 50 kPa
    51, 56, 63, 68, 74, 77, 79, 77, 70, 65,  // 60 kPa
    55, 60, 67, 73, 78, 82, 84, 82, 74, 68,  // 70 kPa
    58, 63, 71, 77, 81, 85, 87, 85, 77, 70,  // 80 kPa
    62, 67, 75, 80, 85, 88, 90, 88, 80, 73,  // 90 kPa
    65, 70, 78, 83, 87, 91, 93, 90, 82, 75   // 100 kPa 
    };

// AFR table
const float afrTable[MAP_BINS * RPM_BINS] = {
    // 600,  1k,   1.5k, 2k,   2.5k, 3k,   3.5k, 4k,   5k,   6k
    14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, // 15 kPa
    14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, // 20 kPa
    14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, // 30 kPa
    14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.7, 14.5, 14.5, // 40 kPa
    14.7, 14.7, 14.7, 14.7, 14.5, 14.5, 14.5, 14.2, 14.0, 14.0, // 50 kPa
    14.5, 14.5, 14.5, 14.5, 14.2, 14.0, 14.0, 13.8, 13.5, 13.5, // 60 kPa
    14.0, 14.0, 14.0, 13.8, 13.5, 13.5, 13.2, 13.0, 13.0, 12.8, // 70 kPa
    13.5, 13.5, 13.5, 13.2, 13.0, 13.0, 12.8, 12.8, 12.5, 12.5, // 80 kPa
    13.0, 13.0, 13.0, 12.8, 12.5, 12.5, 12.5, 12.5, 12.2, 12.2, // 90 kPa
    12.5, 12.5, 12.5, 12.5, 12.5, 12.5, 12.2, 12.2, 12.0, 12.0  // 100 kPa
};

// TODO: Impliment AFR Table
        if(MAPKPA  >= mapAxis[i] ){         // Check if we are in the correct bin
            MAPBin = (i == 0) ? 0 : i - 1;  // Make sure the bin isnt negative
            break;
        }
    }

    for(int i = 0; i <= RPM_BINS; i++){     // Calculate bin of RPM
        if(eng->RPM >= rpmAxis[i] ){        // Check if we are in the correct bin
            RPMBin = (i == 0) ? 0 : i - 1;  // Make sure the bin isnt nagative
            break;
        }
    }

    int AFRIndex = (MAPBin * RPM_BINS) + RPMBin;
    eng->AFR_TARGET = afrTable[AFRIndex];
}

void calculateVE(struct Engine *eng){        // Calculate the engine VE
    int MAPBin = MAP_BINS - 1;
    int RPMBin = RPM_BINS - 1;

    word16 MAPKPA = eng->MAP / 100;

    for(int i = 0; i <= MAP_BINS; i++){     // Calculate bin of MAP
        if(MAPKPA  >= mapAxis[i] ){         // Check if we are in the correct bin
            MAPBin = (i == 0) ? 0 : i - 1;  // Make sure the bin isnt negative
            break;
        }
    }

    for(int i = 0; i <= RPM_BINS; i++){     // Calculate bin of RPM
        if(eng->RPM >= rpmAxis[i] ){        // Check if we are in the correct bin
            RPMBin = (i == 0) ? 0 : i - 1;  // Make sure the bin isnt nagative
            break;
        }
    }

    int VEIndex = (MAPBin * RPM_BINS) + RPMBin;     // Calculate VE value location in 1D map using calculated bins
    eng->VE = VETable[VEIndex];                     // Return VE value fetched from table

}


void calculateFuelLoad(struct Engine *eng){         // Calculate engine theoretical fuel loading

    word16 flowPerMinTh = eng->RPM * eng->displacementPerRev;                   // Calculate the theoretical air flow per minute
    word16 Density = (eng->MAP * 10 * 1000 ) / (287 * eng->IAT);                // Calculate the current air density and scale by 1000 to keep percision
    word32 realAirFlow = (flowPerMinTh * eng->VE) / 100;                        // Calculate the true air flow factoring in Volumetric efficiency
    word32 realAirMass = realAirFlow * Density;                                 // Calculate the real air mass entering the engine still scaled
    word16 fuelLoad = (realAirMass * 10) / (eng->AFR_TARGET);                   // Calculate fuel load and scale back to Grams/Minute.


    eng->fuelLoad = (word16)fuelLoad;     // cast to int16 and return fuel load in grams per minute
}

word16 correctFuelLoad(struct Engine *eng){
    if(eng->Coldstart == true && eng->COOLANT <= eng->coldCoolant){
        eng->fuelLoad = eng->fuelLoad * coldStartEnrichment;
    } else {
        eng->Coldstart = false;
    }
    if (eng->EngineCranking == true){
        eng->fuelLoad = eng->fuelLoad * crankingEnrichment;
    }
    if (eng->fuelTrim != 1){
        eng->fuelLoad = eng->fuelLoad * eng->fuelTrim;
    }

    //TODO: You will need to condition the function to take the multiplier and convert it into actually how much the fuel load should change
    eng->fuelLoad = eng->fuelLoad * eng->toeEnrichmentMultiplier;

    //TODO: add O2 correction

}

void initValues(struct Engine *eng, struct ECUSchedule *sched){

    eng->AFR_TARGET = onBootAFR;                        // Define AFR Target on boot
    eng->coldCoolant = 100;                             // Tempurature where under is considered cold starting
    eng->displacementPerRev = DISPLACEMENT_PER_REV;     // Set engine displacement
    eng->fuelTrim = initFuelTrim;

    sched->ECULoopSize = loopSize;                      // Set the total loop size before the logic repeats
    sched->ECUStep = 0;
    sched->crankCheckInterval = 5;                      // Interval at which the ECU checks for cranking
    sched->TPSCheckInterval = TPSCheck;

    if (eng->COOLANT < eng->coldCoolant){               // On init determine if engine is cold. If so, set the flag.
        eng->Coldstart = true;
    };

}

void performStep(struct Engine *eng, struct ECUSchedule *sched){
    if (sched->ECUStep % sched->crankCheckInterval == 0){           // Check engine cranking status on interval
        if (eng->RPM < 500){
            eng->EngineCranking = true;
        } else {eng->EngineCranking = false;}
    }

    if (sched->ECUStep % sched->TPSCheckInterval == 0){             // Calculate Toe in Enrichment on schedule
        calculateToeEnrichment(eng);
    }

    calculateVE(eng);                   // Update volumetric efficiency

    calculateAFR(eng);                  // Calculate VE

    calculateFuelLoad(eng);             // Calculate the base fuel load

    correctFuelLoad(eng);               // Adjust fuel load for transient conditions
}

struct Engine engineInstance = {0};         // Instantiate instance of engine values
struct ECUSchedule schedule;                // Instantiate the ECU Schedule

int main(){

    initValues(&engineInstance, &schedule);

    while(1){
        for(schedule.ECUStep = 0; schedule.ECUStep < schedule.ECULoopSize; schedule.ECUStep++){ // Iterate through the ECU loop
            performStep(&engineInstance, &schedule);                                            // take a single step of the loop
        }
    }
}
