#include "tsCommon.h"
#include "tsTransportStream.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[], char *envp[]) {
    // TODO - open file
    fstream stream;
    stream.open("scr/input_files/example_new.ts", ios::in | ios::binary);
    uint8_t packetBuffer[TS::TS_PacketLength];
    uint8_t packetBufferNext[TS::TS_PacketLength];
    TS_PacketHeader PacketHeader;
    TS_PacketHeader PacketHeaderNext;
    TS_AdaptationField PacketAdaptationField;
    bool endPackets = false;
    bool endAssembler = false;
    PES_Assembler PESAssembler136;
    PESAssembler136.Init(136);
    PES_Assembler PESAssembler174;
    PESAssembler174.Init(174);
    int32_t PacketId = 0;
    if (!stream.good()) {
        cout << "Error loading the file.";
        return 0;
    }


    while (!stream.eof()) {

        stream.read((char *) packetBuffer, TS::TS_PacketLength);
        PacketHeader.Parse(packetBuffer);

        printf("%010d ", PacketId);
        PacketHeader.Print();



        //Check Adaptation Field Control
        if (PacketHeader.hasAdaptationField()) {
            PacketAdaptationField.Parse(packetBuffer);
            PacketAdaptationField.Print();
        }
        if (PacketHeader.getPID() == 136) {
            endAssembler = (endPackets or PacketHeaderNext.getPayloadUnitStartIndicator());
            PES_Assembler::eResult Result = PESAssembler136.AbsorbPacket(packetBuffer, &PacketHeader,
                                                                         &PacketAdaptationField);
            //printf("%010d ", PacketId);
            switch (Result) {
                case PES_Assembler::eResult::StreamPackedLost :
                    printf("PcktLost ");
                    break;
                case PES_Assembler::eResult::AssemblingStarted :
                    //printf("Started ");
                    //PESAssembler136.PrintPESH();
                    break;
                case PES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;
                case PES_Assembler::eResult::AssemblingFinished:
                    //printf("Finished PES: PcktLen=%d HeadLen=%d DataLen=%d", PESAssembler136.getNumPacketBytes(),
                          //PESAssembler136.getHeaderLen(), PESAssembler136.getDataLen());
                    break;
            }
            //printf("\n");
        }

        printf("\n");

        PacketId++;
    }
    //printf("%d", PESAssembler136.getBufferSize());
    //fwrite(PESAssembler136.getPacket(),PESAssembler136.getDataInBuffer() , 1, PESAssembler136.file);
    fclose(PESAssembler136.file);
    stream.close();
    return 0;
}