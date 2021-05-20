#pragma once

#include "tsCommon.h"
#include <string>
#include <iostream>

using namespace std;
/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================

class TS {
public:
    static constexpr uint32_t TS_PacketLength = 188;
    static constexpr uint32_t TS_HeaderLength = 4;
    static constexpr uint32_t PES_HeaderLength = 6;
    static constexpr uint32_t BaseClockFrequency_Hz = 90000; //Hz
    static constexpr uint32_t ExtendedClockFrequency_Hz = 27000000; //Hz
    static constexpr uint32_t BaseClockFrequency_kHz = 90; //kHz
    static constexpr uint32_t ExtendedClockFrequency_kHz = 27000; //kHz
    static constexpr uint32_t BaseToExtendedClockMultiplier = 300;
};

//=============================================================================================================================================================================

class TS_PacketHeader {
public:
    enum class ePID : uint16_t {
        PAT = 0x0000,
        CAT = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT = 0x0010, //DVB specific PID
        SDT = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };

protected:
    uint8_t sync_byte;
    bool transport_error_indicator;
    bool payload_unit_start_indicator;
    bool transport_priority;
    uint16_t PID;
    uint8_t transport_scrambling_control;
    uint8_t adaptation_field_control;
    uint8_t continuity_counter;

public:
    //void     Reset();
//Packt Header Parser
    void Parse(const uint8_t *Input) {
        uint32_t header = _byteswap_ulong(*((uint32_t *) Input));

        sync_byte = (header & 0xFF000000) >> 24;    //8bits
        transport_error_indicator = (header & 0x00800000) >> 23;  //1bit
        payload_unit_start_indicator = (header & 0x00400000) >> 22; //1bit
        transport_priority = (header & 0x00200000) >> 21; //1bit
        PID = (header & 0x001FFF00) >> 8;    //13 bits
        transport_scrambling_control = (header & 0x000000C0) >> 6;    //2 bits
        adaptation_field_control = (header & 0x00000030) >> 4;    //2 bits
        continuity_counter = (header & 0x00000000F);        //4 bits
    }

//Pinter
    void Print() const {
        cout << "TS: ";
        cout << "SB=";
        printf("%d ", getSyncByte());
        cout << "E=";
        //printf("E=");
        printf("%d ", getTransportErrorIndicator());
        cout << "S=";
        printf("%d ", getPayloadUnitStartIndicator());
        cout << "F=";
        printf("%d ", getTransportPriority());
        cout << "PID=";
        printf("%d ", getPID());
        cout << "TSC=";
        printf("%d ", getTransportScramblingControl());
        cout << "AF=";
        printf("%d ", getAdaptationFieldControl());
        cout << "CC=";
        printf("%d ", getContinuityCounter());
    }

//Get value
    uint8_t getSyncByte() const { return sync_byte; }

    bool getTransportErrorIndicator() const { return transport_error_indicator; }

    bool getPayloadUnitStartIndicator() const { return payload_unit_start_indicator; }

    bool getTransportPriority() const { return transport_priority; }

    uint16_t getPID() const { return PID; }

    uint8_t getTransportScramblingControl() const { return transport_scrambling_control; }

    uint8_t getAdaptationFieldControl() const { return adaptation_field_control; }

    uint8_t getContinuityCounter() const { return continuity_counter; }

public:
    //TODO
    bool hasAdaptationField() const {
        if ((getAdaptationFieldControl() == 2) || (getAdaptationFieldControl() == 3)) {
            return true;
        }
        return false;
    }
    //bool     hasPayload        () const { /*TODO*/ }
};

class TS_AdaptationField {
protected:
    uint8_t AFLength;
    bool discontinuity_indicator;
    bool random_access_indicator;
    bool stream_priority_indicator;
    bool PCR;
    bool OPCR;
    bool SP;
    bool TPD;
    bool AFExt;
    long long int PCR_data;
    long long int OPCR_data;
    double Time;
    int Stuffing;

public:
//Adaptation Field Parser
    void Parse(const uint8_t *Input) {
        AFLength = Input[4];
        Stuffing = AFLength - 1;
        discontinuity_indicator = (Input[5] & 0x80) >> 7;
        random_access_indicator = (Input[5] & 0x40) >> 6;
        stream_priority_indicator = (Input[5] & 0x20) >> 5;
        PCR = (Input[5] & 0x10) >> 4;
        OPCR = (Input[5] & 0x08) >> 3;
        SP = (Input[5] & 0x04) >> 2;
        TPD = (Input[5] & 0x02) >> 1;
        AFExt = (Input[5] & 0x01);
        if (getPCR()) {
            uint64_t tmp = xSwapBytes64(*((uint64_t *) &Input[6]));
            uint64_t basic = (tmp & 0xFFFFFFFF80000000) >> 31;
            uint64_t extension = (tmp & 0x1FF0000) >> 16;
            PCR_data = basic * 300 + extension;
            Time = PCR_data / static_cast<long double>(TS::ExtendedClockFrequency_Hz);
            Stuffing -= 6;
        }
        if (getOPCR()) {
            uint64_t tmp = xSwapBytes64(*((uint64_t *) &Input[6 + getPCR() * 6]));
            OPCR_data = (tmp & 0xFFFFFFFFFFFF0000) >> 16;
            Stuffing -= 6;
        }


    }

//Get value
    uint8_t getAFLength() const { return AFLength; }

    bool getDC() const { return discontinuity_indicator; }

    bool getRA() const { return random_access_indicator; }

    bool getSPI() const { return stream_priority_indicator; }

    bool getPCR() const { return PCR; }

    bool getOPCR() const { return OPCR; }

    bool getSP() const { return SP; }

    bool getTPD() const { return TPD; }

    bool getAFExt() const { return AFExt; }

    int getPCR_data() const { return PCR_data; }

    int getOPCR_data() const { return OPCR_data; }

    double getTime() const { return static_cast<double>(Time); }

    int getStuffing() const { return Stuffing; }

//Printer
    void Print() const {
        //printf("AF: ");
        //printf("L=");
        //if(!getPCR()) return;
        cout << "AF: L=";
        printf("%d ", getAFLength());
        //printf("DC=");
        cout << "DC=";
        printf("%d ", getDC());
        //printf("RA=");
        cout << "RA=";
        printf("%d ", getRA());
        //printf("SP=");
        cout << "SP=";
        printf("%d ", getSPI());
        //printf("PR=");
        cout << "PR=";
        printf("%d ", getPCR());
        //printf("OR=");
        cout << "OR=";
        printf("%d ", getOPCR());
        //printf("SP=");
        cout << "SP=";
        printf("%d ", getSP());
        //printf("TP=");
        cout << "TP=";
        printf("%d ", getTPD());
        //printf("EX=");
        cout << "EX=";
        printf("%d ", getAFExt());
        if (getPCR()) {
            cout << "PCR=";
            printf("%lld ", getPCR_data());
            cout << "(Time=";
            printf("%lf", getTime());
            cout << "s) ";
        }
        if (getOPCR()) {
            cout << "OPCR=";
            printf("%lld ", getOPCR_data());
        }
        cout << "Stuffing=";
        printf("%d", getStuffing());
        //printf("\n");
    }
};

class PES_PacketHeader {
public:
    enum eStreamId : uint8_t {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8,
    };
protected:
//PES packet header
    uint32_t m_PacketStartCodePrefix;
    uint8_t m_StreamId;
    uint16_t m_PacketLength;
public:
    void Reset();

    int32_t Parse(const uint8_t *Packet, const TS_PacketHeader &PacketHeader, const TS_AdaptationField &PacketAP) {
        if (PacketHeader.getPayloadUnitStartIndicator()) {
            uint64_t tmp = xSwapBytes64(*((uint64_t *) &Packet[5 + PacketAP.getAFLength()]));
            //uint64_t tmp = *((uint64_t *) Packet);
            m_PacketStartCodePrefix = (tmp & 0xFFFFFF0000000000) >> 40;
            m_StreamId = (tmp & 0x000000FF00000000) >> 32;
            m_PacketLength = (tmp & 0x00000000FFFF0000) >> 16;
        }
        return 0;
    };

    void Print() const {
        cout << "PES: PSCP=";
        printf("%lld ", getPacketStartCodePrefix());
        //printf("%lld ", 1);
        //printf("DC=");
        cout << "SID=";
        printf("%lld ", getStreamId());
        //printf("RA=");
        cout << "L=";
        printf("%lld ", getPacketLength());
    };
public:
//PES packet header
    uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }

    uint8_t getStreamId() const { return m_StreamId; }

    uint16_t getPacketLength() const { return m_PacketLength; }
};

class PES_Assembler {
public:
    enum class eResult : int32_t {
        UnexpectedPID = 1,
        StreamPackedLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
    };
protected:
//setup
    int32_t m_PID;
//buffer
    uint8_t *m_Buffer;
    uint32_t m_BufferSize;
    uint32_t m_DataOffset;
//operation
    int8_t m_LastContinuityCounter;
    bool m_Started = false;
    PES_PacketHeader m_PESH;
public:
    PES_Assembler() {};

    ~PES_Assembler() {};

    void Init(int32_t PID);

    eResult AbsorbPacket(const uint8_t *TransportStreamPacket, const TS_PacketHeader *PacketHeader,
                         const TS_AdaptationField *AdaptationField) {
        if (PacketHeader->getPID() == 136) {
            m_PESH.Parse(TransportStreamPacket, *PacketHeader, *AdaptationField);
            if (PacketHeader->getPayloadUnitStartIndicator()) {
                m_Started = true;
                m_DataOffset = m_PESH.getPacketLength();
                return eResult::AssemblingStarted;
            } else {
                if(PacketHeader->hasAdaptationField()){
                    //m_PESH.Print();
                    if(m_PESH.getPacketLength()==m_DataOffset) {
                        m_DataOffset+=6;
                        return eResult::AssemblingFinished; }
                }
                return eResult::AssemblingContinue;
            }
        } else return eResult::UnexpectedPID;
        return eResult::AssemblingStarted;
    };

    void PrintPESH() const { m_PESH.Print(); }

    uint8_t *getPacket() { return m_Buffer; }

    int32_t getNumPacketBytes() const { return m_DataOffset; }

protected:
    void xBufferReset();

    void xBufferAppend(const uint8_t *Data, int32_t Size);
};
//=============================================================================================================================================================================
