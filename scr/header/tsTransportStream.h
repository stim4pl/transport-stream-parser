#pragma once
#include "tsCommon.h"
#include <string>

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

    void Print() const
    {
        printf("TS: ");
        printf("SB=");
        printf("%d ", getSyncByte());
        printf("E=");
        printf("%d ", getTransportErrorIndicator());
        printf("S=");
        printf("%d ", getPayloadUnitStartIndicator());
        printf("F=");
        printf("%d ", getTransportPriority());
        printf("PID=");
        printf("%d ", getPID());
        printf("TSC=");
        printf("%d ", getTransportScramblingControl());
        printf("AF=");
        printf("%d ", getAdaptationFieldControl());
        printf("CC=");
        printf("%d ", getContinuityCounter());
    }
    public:
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
        //bool     hasAdaptationField() const { /*TODO*/ }
        //bool     hasPayload        () const { /*TODO*/ }
};

//=============================================================================================================================================================================
