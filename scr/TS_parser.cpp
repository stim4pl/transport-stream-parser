#include "tsCommon.h"
#include "tsTransportStream.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[], char *envp[]) {
    freopen("out.txt", "w", stdout); //Save output to file
    fstream stream;
    stream.open("scr/input_files/example_new.ts", ios::in | ios::binary);
    uint8_t packetBuffer[TS::TS_PacketLength];
    TS_PacketHeader PacketHeader;
    TS_AdaptationField PacketAdaptationField;
    PES_Assembler PESAssembler136;
    PESAssembler136.Init(136);
    PES_Assembler PESAssembler174;
    PESAssembler174.Init(174);
    PES_Assembler::eResult Result;
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


        if (PacketHeader.hasAdaptationField()) {
            PacketAdaptationField.Parse(packetBuffer);
            PacketAdaptationField.Print();
        }
        if (PacketHeader.getPID() == 136) {
            Result = PESAssembler136.AbsorbPacket(packetBuffer, &PacketHeader,
                                                  &PacketAdaptationField);
        }
        if (PacketHeader.getPID() == 174)
            Result = PESAssembler174.AbsorbPacket(packetBuffer, &PacketHeader,
                                                  &PacketAdaptationField);
        if((PacketHeader.getPID() == 136) or (PacketHeader.getPID() == 174)){
            switch (Result) {
                case PES_Assembler::eResult::StreamPackedLost :
                    printf("PcktLost ");
                    break;
                case PES_Assembler::eResult::AssemblingStarted :
                    printf("Started ");
                    if (PacketHeader.getPID() == 136) PESAssembler136.PrintPESH();
                    if (PacketHeader.getPID() == 174) PESAssembler174.PrintPESH();
                    break;
                case PES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;
                case PES_Assembler::eResult::AssemblingFinished:
                    printf("Finished PES: PcktLen=%d HeadLen=%d DataLen=%d ", PESAssembler136.getBufferSize(),
                           PESAssembler136.getHeaderLen(),
                           PESAssembler136.getBufferSize() - PESAssembler136.getHeaderLen());
                    break;
                case PES_Assembler::eResult::UnexpectedPID:
                    break;
                default:
                    break;
            }
        }
        printf("\n");


        PacketId++;
    }
    PESAssembler136.write();
    PESAssembler174.write();
    fclose(PESAssembler136.file);
    fclose(PESAssembler174.file);
    stream.close();
    return 0;
}