#pragma once

#include "tsCommon.h"
#include <string>
#include <iostream>
#include <fstream>

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
    bool hasAdaptationField() const {
        if ((getAdaptationFieldControl() == 2) || (getAdaptationFieldControl() == 3)) {
            return true;
        }
        return false;
    }

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
        cout << "AF: L=";
        printf("%d ", getAFLength());
        cout << "DC=";
        printf("%d ", getDC());
        cout << "RA=";
        printf("%d ", getRA());
        cout << "SP=";
        printf("%d ", getSPI());
        cout << "PR=";
        printf("%d ", getPCR());
        cout << "OR=";
        printf("%d ", getOPCR());
        cout << "SP=";
        printf("%d ", getSP());
        cout << "TP=";
        printf("%d ", getTPD());
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
        printf("%d ", getStuffing());
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
    uint8_t m_HeaderLenght;
    uint8_t m_Header_Data_Lenght;
    bool PTS_Flag = false;
    bool DTS_Flag = false;
    uint64_t PTS;
    uint64_t DTS;
public:
    void Reset();

    int32_t Parse(const uint8_t *Packet) {
        uint64_t tmp = xSwapBytes64(*((uint64_t *) &Packet[0]));
        m_PacketStartCodePrefix = (tmp & 0xFFFFFF0000000000) >> 40;
        m_StreamId = (tmp & 0x000000FF00000000) >> 32;
        m_PacketLength = (tmp & 0x00000000FFFF0000) >> 16;
        m_HeaderLenght = 6;
        if (m_StreamId != eStreamId::eStreamId_program_stream_map,
                m_StreamId != eStreamId::eStreamId_padding_stream,
                m_StreamId != eStreamId::eStreamId_private_stream_2,
                m_StreamId != eStreamId::eStreamId_ECM,
                m_StreamId != eStreamId::eStreamId_EMM,
                m_StreamId != eStreamId::eStreamId_program_stream_directory,
                m_StreamId != eStreamId::eStreamId_DSMCC_stream,
                m_StreamId != eStreamId::eStreamId_ITUT_H222_1_type_E) {
            m_Header_Data_Lenght = Packet[6 + 2];
            m_HeaderLenght += 3 + m_Header_Data_Lenght;
            uint8_t PTSDTSFlag = (Packet[6 + 1] & 0xC0) >> 6;
            switch (PTSDTSFlag) {
                case 0: {
                    PTS = 0;
                    DTS = 0;
                }
                    break;
                case 1:
                    break;
                case 2: {
                    uint64_t tmp1 = xSwapBytes64(*((uint64_t *) &Packet[+6 + 3]));
                    tmp1 = tmp1 >> 24;
                    uint64_t tmp2 = (tmp1 & 0xE00000000) >> 3;
                    int64_t tmp3 = (tmp1 & 0xFFFE0000) >> 2;
                    int64_t tmp4 = (tmp1 & 0xFFFE) >> 1;
                    PTS = (tmp2 | tmp3 | tmp4);
                    PTS_Flag = true;
                }
                    break;
                case 3: {
                    uint64_t tmp1 = xSwapBytes64(*((uint64_t *) &Packet[6 + 3]));
                    tmp1 = tmp1 >> 24;
                    uint64_t tmp2 = (tmp1 & 0xE00000000) >> 3;
                    int64_t tmp3 = (tmp1 & 0xFFFE0000) >> 2;
                    int64_t tmp4 = (tmp1 & 0xFFFE) >> 1;
                    PTS = (tmp2 | tmp3 | tmp4);
                    PTS_Flag = true;

                    tmp1 = xSwapBytes64(*((uint64_t *) &Packet[6 + 3 + 5]));
                    tmp1 = tmp1 >> 24;
                    tmp2 = (tmp1 & 0xE00000000) >> 3;
                    tmp3 = (tmp1 & 0xFFFE0000) >> 2;
                    tmp4 = (tmp1 & 0xFFFE) >> 1;
                    DTS = (tmp2 | tmp3 | tmp4);
                    DTS_Flag = true;
                }
                    break;
            }
        }
        return 0;
    };

    void Print() const {
        cout << "PES: PSCP=";
        printf("%lld ", getPacketStartCodePrefix());
        cout << "SID=";
        printf("%lld ", getStreamId());
        cout << "L=";
        printf("%lld ", getPacketLength());
        if (PTS_Flag)printf("PTS= %lld ", PTS);
        if (PTS_Flag)printf("(Time= %lfs)", static_cast<double>(PTS) / TS::BaseClockFrequency_Hz);
        if (DTS_Flag)printf("DTS= %lld ", DTS);
        if (DTS_Flag)printf("(Time= %lfs)", static_cast<double>(DTS) / TS::BaseClockFrequency_Hz);
    };
public:
//PES packet header
    uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }

    uint8_t getStreamId() const { return m_StreamId; }

    uint16_t getPacketLength() const { return m_PacketLength; }

    uint8_t getHeaderLen() const { return m_HeaderLenght; }
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
    FILE *file;
protected:
//setup
//buffer
    uint8_t *m_Buffer = new uint8_t[0];
    uint8_t *m_BufferTmp = new uint8_t[0];
    uint32_t m_DataInBuffor = 0;
    uint8_t m_HeaderLen;
    uint32_t m_PacketLen;
    uint32_t m_BufferSize = 0;

//operation
    int8_t m_LastContinuityCounter = 0;
    bool m_Started = false;
    PES_PacketHeader m_PESH;
public:
    int32_t m_PID;
    PES_Assembler() {};

    ~PES_Assembler() {};

    void Init(int32_t PID) {
        m_PID = PID;
        if (m_PID == 136) file = fopen("pid136.mp2", "ab");
        else if (m_PID == 174) file = fopen("pid174.264", "ab");
    };

    eResult AbsorbPacket(const uint8_t *TransportStreamPacket, const TS_PacketHeader *PacketHeader,
                         const TS_AdaptationField *AdaptationField) {
        if (PacketHeader->getPID() == m_PID) {
            if (PacketHeader->getPayloadUnitStartIndicator()) {
                if (m_Started) {
                    m_Started = false;
                    if (m_PID == 174)
                        printf(" PES: Previous was Finished of PES, PcktLen=%d HeadLen=%d DataLen=%d ", getBufferSize(),
                               getHeaderLen(), getBufferSize() - getHeaderLen());
                    write();
                }

                if (!m_Started) {
                    m_Started = true;
                    xBufferReset();
                    if (PacketHeader->getAdaptationFieldControl() == 1)
                        xBufferAppend(TransportStreamPacket, TS::TS_HeaderLength);
                    if (PacketHeader->getAdaptationFieldControl() == 3 and AdaptationField->getAFLength() < 183)
                        xBufferAppend(TransportStreamPacket, TS::TS_HeaderLength +
                                                             1 + AdaptationField->getAFLength());
                    m_PESH.Parse(m_Buffer);
                    m_HeaderLen = m_PESH.getHeaderLen();
                    m_PacketLen = m_PESH.getPacketLength();
                    return eResult::AssemblingStarted;
                }
            } else {
                if (PacketHeader->getAdaptationFieldControl() == 1)
                    xBufferAppend(TransportStreamPacket, TS::TS_HeaderLength);
                if (PacketHeader->getAdaptationFieldControl() == 3 and AdaptationField->getAFLength() < 183) {
                    xBufferAppend(TransportStreamPacket, TS::TS_HeaderLength +
                                                         1 + AdaptationField->getAFLength());
                }
                if ((m_PID == 136) and ((m_PacketLen + 6 - m_HeaderLen) == (getBufferSize() - getHeaderLen())))
                    return eResult::AssemblingFinished;
                m_LastContinuityCounter++;
                return eResult::AssemblingContinue;
            }
        } else return eResult::UnexpectedPID;
        return eResult::AssemblingStarted;
    };

    void PrintPESH() const { m_PESH.Print(); }

    int8_t getHeaderLen() const { return m_HeaderLen; }

    uint32_t getBufferSize() const { return m_BufferSize; }

    void write() { fwrite(m_Buffer + m_HeaderLen, m_DataInBuffor - m_HeaderLen, 1, file); }

protected:
    void xBufferReset() {
        m_BufferSize = 0;
        m_DataInBuffor = 0;
        delete[] m_Buffer;
        m_Buffer = new uint8_t[0];
    };

    void xBufferAppend(const uint8_t *Data, int32_t Size) {

        m_BufferSize += TS::TS_PacketLength - Size;
        m_BufferTmp = new uint8_t[m_BufferSize];
        copy(m_Buffer, m_Buffer + m_DataInBuffor, m_BufferTmp);
        delete[] m_Buffer;
        m_Buffer = m_BufferTmp;
        copy(Data + Size, Data + TS::TS_PacketLength, m_Buffer + m_DataInBuffor);
        m_DataInBuffor += TS::TS_PacketLength - Size;
    };
};
//=============================================================================================================================================================================
