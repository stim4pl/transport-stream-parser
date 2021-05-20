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

class TS
{
public:
  static constexpr uint32_t TS_PacketLength = 188;
  static constexpr uint32_t TS_HeaderLength = 4;
  static constexpr uint32_t PES_HeaderLength = 6;
  static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};

//=============================================================================================================================================================================

class TS_PacketHeader
{
public:
  enum class ePID : uint16_t
  {
    PAT  = 0x0000,
    CAT  = 0x0001,
    TSDT = 0x0002,
    IPMT = 0x0003,
    NIT  = 0x0010, //DVB specific PID
    SDT  = 0x0011, //DVB specific PID
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
    void Parse(const uint8_t* Input)
    {
        uint32_t header = _byteswap_ulong(*((uint32_t*)Input));

        sync_byte = (header & 0xFF000000) >> 24;	//8bits
        transport_error_indicator = (header & 0x00800000) >> 23;  //1bit
        payload_unit_start_indicator = (header & 0x00400000) >> 22; //1bit
        transport_priority = (header & 0x00200000) >> 21; //1bit
        PID = (header & 0x001FFF00) >> 8;	//13 bits
        transport_scrambling_control = (header & 0x000000C0) >> 6;	//2 bits
        adaptation_field_control = (header & 0x00000030) >> 4;	//2 bits
        continuity_counter = (header & 0x00000000F);		//4 bits
    }
//Pinter
    void Print() const
    {
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
        if ((getAdaptationFieldControl() == 2) || (getAdaptationFieldControl() == 3))
        {
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
    void Parse(const uint8_t* Input) {
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
        if(getPCR())
        {
            uint64_t tmp = xSwapBytes64(*((uint64_t*)&Input[6]));
            uint64_t basic = (tmp & 0xFFFFFFFF80000000) >> 31;
            uint64_t extension = (tmp & 0x1FF0000) >> 16;
            PCR_data = basic * 300 + extension;
            Time = PCR_data / static_cast<long double>(TS::ExtendedClockFrequency_Hz);
            Stuffing -= 6;
        }
        if(getOPCR())
        {
            uint64_t tmp = xSwapBytes64(*((uint64_t*)&Input[6 + getPCR() * 6]));
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
    int getStuffing() const { return Stuffing;}
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
        if(getPCR())
        {
            cout << "PCR=";
            printf("%lld ", getPCR_data());
            cout << "(Time=";
            printf("%lf", getTime());
            cout << "s) ";
        }
        if(getOPCR())
        {
            cout << "OPCR=";
            printf("%lld ", getOPCR_data());
        }
        cout << "Stuffing=";
        printf("%d", getStuffing());
        //printf("\n");
    }
};
//=============================================================================================================================================================================
