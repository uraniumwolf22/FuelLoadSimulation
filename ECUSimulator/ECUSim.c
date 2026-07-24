#include <stdio.h>
#include <stdbool.h>
#include "tables.h"

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

    word16 MAPKPA = eng->MAP / 100;

    for(int i = 0; i < MAP_BINS; i++){
        if(MAPKPA >= mapAxis[i]){
            MAPBin = (i == 0) ? 0 : i - 1;
            break;
        }
    }

    for(int i = 0; i < RPM_BINS; i++){
        if(eng->RPM >= rpmAxis[i]){
            RPMBin = (i == 0) ? 0 : i - 1;
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
    eng->IAT = KtoFConversion(70);                             // Set intake air tempurature to 70F on boot

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

struct Engine* get_engine_state() {     // To allow python to get the engine state
    return &engineInstance;
}

int main(){

    initValues(&engineInstance, &schedule);

    while(1){
        for(schedule.ECUStep = 0; schedule.ECUStep < schedule.ECULoopSize; schedule.ECUStep++){ // Iterate through the ECU loop
            performStep(&engineInstance, &schedule);                                            // take a single step of the loop
        }
    }
}
