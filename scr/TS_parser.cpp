#include "tsCommon.h"
#include "tsTransportStream.h"
#include <fstream>
//#include <iostream>
using namespace std;
int main( int argc, char *argv[ ], char *envp[ ])
{
    // TODO - open file
    fstream stream;
    stream.open("scr/input_files/example_new.ts", ios::in | ios::binary);
    uint8_t packetBuffer[TS::TS_PacketLength];
    TS_PacketHeader  PacketHeader;
    TS_AdaptationField PacketAdaptationField;
    int32_t PacketId = 0;
    if(!stream.good())
    {
        cout << "Error loading the file.";
        return 0;
    }

    while(!stream.eof())
    {
        stream.read((char*)packetBuffer,TS::TS_PacketLength);
        //TS_PacketHeader.Reset();
        //Parse Packet Header
        PacketHeader.Parse(packetBuffer);

        //Print data Header
        printf("%010d ", PacketId);
        PacketHeader.Print();

        //Check Adaptation Field Control
        if (PacketHeader.hasAdaptationField())
        {
            //Parse Packet Adaptation Field
            PacketAdaptationField.Parse(packetBuffer);
            //Print data Adaptation Field
            PacketAdaptationField.Print();
        }

        printf("\n");

        PacketId++;
    }
    stream.close();
    return 0;
}