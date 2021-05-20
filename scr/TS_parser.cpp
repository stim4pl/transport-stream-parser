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
    //PES_PacketHeader PacketPES;
    PES_Assembler PESAssembler;
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
            //printf("\n");
        }
        if(PacketHeader.getPID() == 136){
            //PacketPES.Parse(packetBuffer, PacketHeader, PacketAdaptationField);
            //PacketPES.Print();
            PES_Assembler::eResult Result = PESAssembler.AbsorbPacket(packetBuffer, &PacketHeader, &PacketAdaptationField);
            switch (Result) {
                case PES_Assembler::eResult::StreamPackedLost : printf("PcktLost "); break;
                case PES_Assembler::eResult::AssemblingStarted : printf("Started "); PESAssembler.PrintPESH(); break;
                case PES_Assembler::eResult::AssemblingContinue: printf("Continue "); break;
                case PES_Assembler::eResult::AssemblingFinished: printf("Finished "); printf("PES: Len=%d", PESAssembler.getNumPacketBytes()); break;
            }
            printf("xd");
        }

        printf("\n");

        PacketId++;
    }
    stream.close();
    return 0;
}