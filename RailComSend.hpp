/** \copyright
 * Copyright (c) 2014, Balazs Racz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are  permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file RailComSend.hpp
 * Helper functions and tables for RailCom implementations.
 *
 * - 07. Feb 2025 - Refactoring for "send only" by Olivier Chatelain
 * - 12. Nov 2014 - Initial version             by Balazs Racz
 * 
 */

#ifndef _DCC_RAILCOM_HPP_
#define _DCC_RAILCOM_HPP_

#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <cstdint>


#define RAILCOM_DELAY_CH1  80
#define RAILCOM_DELAY_CH2 193

namespace railcom
{
    /// Code point for ACK (according to RCN-217)
    static constexpr uint8_t ACK1 = 0x0F;
    static constexpr uint8_t ACK2 = 0xF0;
    /// Code point for NACK (according to RCN-217)
    static constexpr uint8_t NACK = 0x3C;
    /// Code point for BUSY (according to NMRA S-9.3.2)
    static constexpr uint8_t BUSY = 0xE1;    

    /// Table for 6-to-8 encoding of railcom data. The table can be indexed by a
    /// 6-bit value that is the semantic content of a railcom byte, and returns the
    /// matching 8-bit value to put out on the UART. This table only contains the
    /// standard codes, for ACK, NACK, BUSY see above.
    const uint8_t railcom_encode[64] = {
        //0x0x   1     2     3     4     5     6     7
        0xAC, 0xAA, 0xA9, 0xA5, 0xA3, 0xA6, 0x9C, 0x9A,
        // 8     9     A     B     C     D     E     F
        0x99, 0x95, 0x93, 0x96, 0x8E, 0x8D, 0x8B, 0xB1,
        //0x1x   1     2     3     4     5     6     7
        0xB2, 0xB4, 0xB8, 0x74, 0x72, 0x6C, 0x6A, 0x69,
        // 8     9     A     B     C     D     E     F
        0x65, 0x63, 0x66, 0x5C, 0x5A, 0x59, 0x55, 0x53,
        //0x2x   1     2     3     4     5     6     7
        0x56, 0x4E, 0x4D, 0x4B, 0x47, 0x71, 0xE8, 0xE4,
        // 8     9     A     B     C     D     E     F
        0xE2, 0xD1, 0xC9, 0xC5, 0xD8, 0xD4, 0xD2, 0xCA,
        //0x3x   1     2     3     4     5     6     7
        0xC6, 0xCC, 0x78, 0x17, 0x1B, 0x1D, 0x1E, 0x2E,
        // 8     9     A     B     C     D     E     F
        0x36, 0x3A, 0x27, 0x2B, 0x2D, 0x35, 0x39, 0x33,
    };

    uint8_t encode6to8(uint8_t data)
    {
        return railcom_encode[data & 0x3F];
    }

    uint8_t decode8to6(uint8_t data) 
    {
        for (uint8_t i = 0; i < 64; i++) {
            if (railcom_encode[i] == data) {
                return i;
            }
        }
        return 0xFF;
    }

    uint16_t decode16to12(uint16_t data) {
        uint32_t payload = 0;
        for(int i = 0; i < 2; i++) {
            payload |= decode8to6(data >> (i * 8) & 0xFF ) << (i * 6);
        }
        return payload;
    }

    uint32_t decode24to18(uint32_t data) 
    {
        uint32_t payload = 0;
        for(int i = 0; i < 3; i++) {
            payload |= decode8to6(data >> (i * 8) & 0xFF ) << (i * 6);
        }
        return payload;
    }

    /// Encodes 12 bits of useful payload into 16 bits of UART data to transmit.
    /// @param id top 4 bits of the payload to send
    /// @param data bottom 8 bits of payload to send.
    /// @return the uart bytes, first byte in the high 8 bits, second byte in
    /// the low 8 bits.
    uint16_t encode12to16(uint8_t id, uint8_t payload)
    {
        return ( ( encode6to8( (id << 2) | (payload >> 6)) << 8) // ID(4 Bit) + 2 MSB of Data(2 Bit)
                 | encode6to8(payload) );                        //             6 LSB of Data
    }

    /// Hamming encode 24 bits UART data from 18 bits payload
    uint32_t encode18to24(uint8_t id, uint16_t payload)
    {
        return ( ( encode6to8( (id << 2) | (payload >> 12)) << 16 ) // Payload bits 16..32 used
                 | encode6to8(payload >> 6) << 8
                 | encode6to8(payload)
                );
    }

    /// Hamming encode 32 bits UART data from 24 bits payload
    uint32_t encode24to32(uint8_t id, uint32_t payload)
    {
        return ( ( encode6to8( (id << 2) | (payload >> 18)) << 24 ) // Payload bits 16..32 used
                 | encode6to8(payload >> 12) << 16
                 | encode6to8(payload >>  6) <<  8
                 | encode6to8(payload)
                );
    }

    uint16_t sendShortAddress(uint8_t cv1_short_address, uint8_t cycle) {

        uint8_t part = cycle % 2;
        switch(part) {
            case 0:
                // Send the first part of the short address
                return encode12to16(0x1, 0);
                break;
                
            case 1:
                // Send the second part of the short address
                return encode12to16(0x2, cv1_short_address);
                break;
            default:
                return NACK;
            }
    }

    uint16_t sendLongAddress( uint8_t cv17_long_addres_high, uint8_t cv18_long_addres_low, uint8_t cycle) {

        uint8_t part = cycle % 2;
        switch(part) {
            case 0:
                // Send the MSB of the long address
                return encode12to16(0x1, 0xBF & cv17_long_addres_high); // Mask bit 7:6 according to RCN-217
                break;
                
            case 1:
                // Send the LSB of the long address
                return encode12to16(0x2, cv18_long_addres_low);
                break;
            default:
                return NACK;
            }
    }
    /*
        /// Hamming encode 40 bits UART data from 36 bits payload
        static uint64_t encode36to48(uint8_t id, uint64_t payload)
        {
            return ( ( encode6to8( (id << 2) | (payload >> 30)) << 40 ) // Payload bits 16..32 used
                    | encode6to8(payload >> 24) << 32
                    | encode6to8(payload >> 18) << 24
                    | encode6to8(payload >> 12) << 16
                    | encode6to8(payload >>  6) <<  8
                    | encode6to8(payload)
                    );
        }
    */

    //
    // See: https://docs.tcsdcc.com/wiki/CV_28
    typedef enum
    {
        CV28_CHANNEL1_ACTIVE    = 1 << 0,  // - Bit 0: Channel 1 is active
        CV28_CHANNEL2_ACTIVE    = 1 << 1,  // - Bit 1: Channel 2 is active
        CV28_CHANNEL2_AUTO_OFF  = 1 << 2,  // - Bit 2: Channel 2 Auto-Off
        CV28_CHANNEL1_WITH_ID3  = 1 << 3,  // - Bit 3: Channel 1 with ID3 (Informations)
        CV28_PROG_ADDR_0003     = 1 << 4,  // - Bit 4: Programming adress '0003'
        CV28_RESERVED_DONT_USE  = 1 << 5,  // - Bit 5: Reserved, not used
        CV28_RAILCOM_HIGH_CURR  = 1 << 6,  // - Bit 6: High-current (60mA) RailCom
        CV28_DCC_AUT_LOGON      = 1 << 7,  // - Bit 7: Allow automatic login according to DCC-A (RCN-218)
    } CV_28_BITS;

    bool CV28_is_ch1_active(uint8_t cv28_railcom)
    {
        return (cv28_railcom & CV28_CHANNEL1_ACTIVE) != 0;
    }

    bool CV28_is_ch2_active(uint8_t cv28_railcom)
    {
        return (cv28_railcom & CV28_CHANNEL2_ACTIVE) != 0;
    }

    bool CV28_is_ch1_auto_off(uint8_t cv28_railcom)
    {
        return (cv28_railcom & 0x4) != 0;
    }

    bool CV28_is_ch1_with_id3(uint8_t cv28_railcom)
    {
        return (cv28_railcom & 0x8) != 0;
    }

    unsigned long getSleepNeeded(unsigned long endBitTime, unsigned long offset, unsigned long now) {
        
        long sleepRest = offset - (now - endBitTime);
        return (sleepRest > 0 ? sleepRest : 0);
    }

}  // namespace railcom

#endif // _DCC_RAILCOM_HPP_
