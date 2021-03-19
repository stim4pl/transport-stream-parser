#include "tsCommon.h"
#include "tsTransportStream.h"
#include <fstream>
#include <iostream>
using namespace std;
int main( int argc, char *argv[ ], char *envp[ ])
{
    // TODO - open file
    fstream stream;
    stream.open("scr/input_files/example_new.ts", ios::in | ios::binary);
    uint8_t packetBuffer[TS::TS_PacketLength];
    TS_PacketHeader  PacketHeader;
    int32_t PacketId = 0;
    if(!stream.good())
    {
        cout << "Error loading the file.";
        return 0;
    }

    while(!stream.eof())
    {
        stream.read((char*)packetBuffer,TS::TS_PacketLength);
        // TODO - read from file

        //TS_PacketHeader.Reset();
        PacketHeader.Parse(packetBuffer);
        printf("%010d ", PacketId);
        PacketHeader.Print();
        printf("\n");

        PacketId++;
    }
    stream.close();
    return 0;
}