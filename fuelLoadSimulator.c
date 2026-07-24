// Sensor values start as 16 bit uint_16 data types
// System is capable of Simple 32 bit math but is very expensive.
/* #################O2 SENSOR##############################
 * O2 sensor is measured from a range of 0 - 20000.
 * A typical AFR would be 14700 or 14.7 to one.
 * Conversion:  AFR_INT = AFR * 1000.
 * This data will be pre-interpolated at the IO level
 *
 * ################MAP VALUES##############################
 * Map values are between 0 and 10000
 * A half throttle value of 50000 pascals would be written as
 * 5000.
 * Conversion: MAP_INT = MAP / 10.  Converted / interpolated at IO level
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>

#define word uint16_t
#define word32 uint32_t


const word DISPLACEMENT = 4;                            // Engine displacement in L
const word DISPLACEMENT_PER_REV = DISPLACEMENT / 2;     // This will be pre-calculated and stored in ROM
const word VE = 100;                                    // Volumetric Efficiency in % rang 0 - 100
const word TARGET_AFR = 150;

const word RPM = 7562;       // Rotations per minute
const word MAP = 12138;      // Manifold air pressure measured in Pa / 10
const word IAT = 223;       // Intake air tempurature in K

const word COOLANT = 200;   // Coolant tempurature in F
const word OXYGEN = 14700;  // Measured AFR from O2 sensor measured in AFR * 1000

word calculateFuelLoad(word RPM,word MAP,word IAT){

    word flowPerMinTh = RPM * DISPLACEMENT_PER_REV;               // Calculate the theoretical air flow per minute
    printf("Theoretical flow per minute is %d L/m\n",flowPerMinTh);

    word Density = (MAP * 10 * 1000 ) / (287 * IAT);                // Calculate the current air density and scale by 1000 to keep percision
    printf("Air density is %d\n",Density);

    word realAirFlow = (flowPerMinTh * VE) / 100;                   // Calculate the true air flow factoring in Volumetric efficiency
    printf("Adjusted air flow is %d L/m\n",realAirFlow);

    word32 realAirMass = realAirFlow * Density;                     // Calculate the real air mass entering the engine still scaled

    printf("Real air mass is %d\n",realAirMass);

    word fuelLoad = (realAirMass * 10) / (TARGET_AFR * 1000);     // Calculate fuel load and scale back to Grams/Minute.
    return (word)fuelLoad;
}

word correctFuelLoad(word fuelLoad){

}

int main(){
    word LD = calculateFuelLoad(RPM,MAP,IAT);
    printf("Fuel load is %d Grams per minute\n",LD);


}
