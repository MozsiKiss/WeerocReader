#!/bin/bash

#If an argument is not given...
if [ "$#" -ne 1 ]; then
    echo ""
    echo "Please specify a Weeroc output file!"
    echo "Usage: > bash $0 FileName(.txt)" >&2
    echo ""
    exit 1
fi
if ! [ -e "$1" ]; then
  echo "$1 not found" >&2
  exit 1
fi

Lines=$(wc -l $1 | awk '{print $1}')

#For skipping header
DataLines=$(echo "$Lines-1" | bc)

#First line in data file needs to be properly formatted header for ROOT
#Argument -n is needed to prevent the inclusion of a newline character
echo -n "hit0/i:ChargeLG0:ChargeHG0:hit1:ChargeLG1:ChargeHG1:hit2:ChargeLG2:ChargeHG2:hit3:ChargeLG3:ChargeHG3:hit4:ChargeLG4:ChargeHG4:hit5:ChargeLG5:ChargeHG5:hit6:ChargeLG6:ChargeHG6:hit7:ChargeLG7:ChargeHG7:hit8:ChargeLG8:ChargeHG8:hit9:ChargeLG9:ChargeHG9:hit10:ChargeLG10:ChargeHG10:hit11:ChargeLG11:ChargeHG11:hit12:ChargeLG12:ChargeHG12:hit13:ChargeLG13:ChargeHG13:hit14:ChargeLG14:ChargeHG14:hit15:ChargeLG15:ChargeHG15:hit16:ChargeLG16:ChargeHG16:hit17:ChargeLG17:ChargeHG17:hit18:ChargeLG18:ChargeHG18:hit19:ChargeLG19:ChargeHG19:hit20:ChargeLG20:ChargeHG20:hit21:ChargeLG21:ChargeHG21:hit22:ChargeLG22:ChargeHG22:hit23:ChargeLG23:ChargeHG23:hit24:ChargeLG24:ChargeHG24:hit25:ChargeLG25:ChargeHG25:hit26:ChargeLG26:ChargeHG26:hit27:ChargeLG27:ChargeHG27:hit28:ChargeLG28:ChargeHG28:hit29:ChargeLG29:ChargeHG29:hit30:ChargeLG30:ChargeHG30:hit31:ChargeLG31:ChargeHG31:temp" > TempFile.txt

#Appends data lines (all lines excluding Weeroc header) to file already containing ROOT-formatted header
$(tail -${DataLines} ${1} >> TempFile.txt)

#Runs ROOT script "WeerocToROOT.cc" and quits when done
root -l -q "WeerocToROOT.cc(\"$1\")"

rm -f TempFile.txt

mv OutputFile.root ${1}.root

echo "Done! Wrote file"
echo "${1}.root"
echo ""

exit
