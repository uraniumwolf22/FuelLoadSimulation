RPM = 500           #Rotations per minute
airTemp = 250       #Air tempurature in Absolute (K)
MAP = 100000        #Air pressure in Pascals
VE = 100             #Engine induction efficiency in %
Displacement = 4    #Engine displacement in Liters
targetAFR = 15    #Target air fuel ratio

tankSize = 24   #Tank size in gallons


#Constant calculations
displacementPerRevolution = Displacement / 2  #Engine displacement per revolution
PreCalculatedVE = VE / 100

#Dynamic
theoreticalAirFlow = displacementPerRevolution * RPM        #Calculate the theoretical air flow from the current RPM and engine size

print(theoreticalAirFlow)
airDensity = MAP / (287 * airTemp)                          #287 is the ideal gas constant for dry air
print(airDensity)
VECorrectedAirFlow = theoreticalAirFlow * PreCalculatedVE   #Air flow adjusted for volumetric efficiency

realAirMassFlow = VECorrectedAirFlow * airDensity
print(realAirMassFlow)
fuelLoad = realAirMassFlow / targetAFR                      #Fuel consumption in g/m

print(fuelLoad)

# #Caclulate run time
# tankSize = 24   #Tank size in gallons

# litersPerMinute = fuelLoad / 750                            #Fuel load in grams per second

# gallonsPerMinute = litersPerMinute * 0.26417205

# gallonsPerHour = gallonsPerMinute * 60

# runTime = tankSize / gallonsPerHour

# print(str(runTime) + "hours till your out of fuel ):")
