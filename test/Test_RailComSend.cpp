
#include "RailComSend.hpp"
#include <gtest/gtest.h>

TEST(DataDecodingTest, Decode8to6) {

  EXPECT_EQ(railcom::decode8to6(0xAC), 0x00);

}

TEST(DataDecodingTest, Decode16to12) {

  EXPECT_EQ(railcom::decode16to12(0x5339), 0x7fe);

  // 5aac8e 5aac8d
  EXPECT_EQ(railcom::decode24to18(0x5aac8e), 0x1c00c); // 0x1c00c = 0111 00 0000 0000 1100 - ID(7) 0000-0000 0011-00
  EXPECT_EQ(railcom::decode24to18(0x5aac8d), 0x1c00d); // 0x1c00d = 0111 00 0000 0000 1101 - ID(7) 0000-0000 0011-01

  // c3ea3f8197
  // b2acac 939c17
  EXPECT_EQ(railcom::decode24to18(0xb2acac), 0x10000); // 0x1c00c = 0001 00 0000 0000 1100 - ID(1) 0000-0000 0011-00
  EXPECT_EQ(railcom::decode24to18(0x939c17), 0x0A1B3); // 0x1c00d = 0000 00 0000 0000 1100 - ID(7) 0000-0000 0011-01

}

TEST(DataEncodingTest, Encode6to8) {

  EXPECT_EQ(railcom::encode6to8(0x00), 0xac);
  EXPECT_EQ(railcom::encode6to8(0x40), 0xAC);
  EXPECT_EQ(railcom::encode6to8(0xC0), 0xAC);

  EXPECT_EQ(railcom::encode6to8(0x01), 0xAA);
  EXPECT_EQ(railcom::encode6to8(0x05), 0xA6);
  EXPECT_EQ(railcom::encode6to8(0x0F), 0xB1);

  EXPECT_EQ(railcom::encode6to8(0x10), 0xB2);
  EXPECT_EQ(railcom::encode6to8(0x30), 0xC6);
  EXPECT_EQ(railcom::encode6to8(0xEF), 0xCA);

  EXPECT_EQ(railcom::encode6to8(0x3F), 0x33);
  EXPECT_EQ(railcom::encode6to8(0x7F), 0x33);
  EXPECT_EQ(railcom::encode6to8(0xFF), 0x33);

}

TEST(DataEncodingTest, Encode12to16) {

  EXPECT_EQ(railcom::encode12to16(0, 0), 0xACAC);
  EXPECT_EQ(railcom::encode12to16(0xF, 0), 0x2DAC);
  EXPECT_EQ(railcom::encode12to16(0x5, 0x55), 0x6C6C);
  EXPECT_EQ(railcom::encode12to16(0x7, 0xFE), 0x5339);
  EXPECT_EQ(railcom::encode12to16(0xF, 0xFF), 0x3333);  
}

TEST(DataEncodingTest, Encode18to24) {
  EXPECT_EQ(railcom::encode18to24(0x0,    0x0), 0xACACAC);
  EXPECT_EQ(railcom::encode18to24(0x1,    0x1), 0xA3ACAA);
  EXPECT_EQ(railcom::encode18to24(0x5, 0x1555), 0x6C6C6C);
  EXPECT_EQ(railcom::encode18to24(0x7, 0x3FFE), 0x533339);
  EXPECT_EQ(railcom::encode18to24(0xF, 0x3FFF), 0x333333);
}

TEST(DataEncodingTest, Encode24to32) {
  EXPECT_EQ(railcom::encode24to32(0x0,     0x0), 0xACACACAC);
  EXPECT_EQ(railcom::encode24to32(0x1,     0x1), 0xA3ACACAA);
  EXPECT_EQ(railcom::encode24to32(0x5, 0x55555), 0x6C6C6C6C);
  EXPECT_EQ(railcom::encode24to32(0x7, 0xFFFFE), 0x53333339);
  EXPECT_EQ(railcom::encode24to32(0xF, 0xFFFFF), 0x33333333);
}

/*
TEST(DataEncodingTest, Encode36to48) {
  EXPECT_EQ(railcom::encode36to48(0x0,     0x0), 0xACACACACACAC);
  // EXPECT_EQ(railcom::encode24to32(0x1,     0x1), 0xA3ACACAA);
  // EXPECT_EQ(railcom::encode24to32(0x5, 0x55555), 0x6C6C6C6C);
  // EXPECT_EQ(railcom::encode24to32(0x7, 0xFFFFE), 0x53333339);
  // EXPECT_EQ(railcom::encode24to32(0xF, 0xFFFFF), 0x33333333);
}
*/
TEST(CV28Test, ChannelOneActive) {
  EXPECT_EQ(railcom::CV28_is_ch1_active( 1), true);
  EXPECT_EQ(railcom::CV28_is_ch1_active( 0), false);
  EXPECT_EQ(railcom::CV28_is_ch1_active(!1), false);
}

TEST(CV28Test, ChannelOneAutoOff) {
  EXPECT_EQ(railcom::CV28_is_ch1_auto_off( 4),   true);
  EXPECT_EQ(railcom::CV28_is_ch1_auto_off( 0),  false);
  EXPECT_EQ(railcom::CV28_is_ch1_auto_off( 1),  false);
  EXPECT_EQ(railcom::CV28_is_ch1_auto_off(!4), false);

  EXPECT_EQ(railcom::CV28_is_ch1_auto_off(0xFF), true);
  EXPECT_EQ(railcom::CV28_is_ch1_auto_off(0xB), false);
}

TEST(CV28Test, ChannelOneWithId3) {
  EXPECT_EQ(railcom::CV28_is_ch1_with_id3( 8),   true);
  EXPECT_EQ(railcom::CV28_is_ch1_with_id3(!8), false);
  EXPECT_EQ(railcom::CV28_is_ch1_with_id3( 0),  false);
}

TEST(CV28Test, ChannelTwoActive) {
  EXPECT_EQ(railcom::CV28_is_ch2_active(2),   true);
  EXPECT_EQ(railcom::CV28_is_ch2_active(!2), false);
  EXPECT_EQ(railcom::CV28_is_ch2_active(0),  false);
}
  
TEST(AddressBroadcasetTest, ShortAddressCh1) {
  EXPECT_EQ(railcom::sendShortAddress( 66, 0 ), 0xA3AC ); // Send short address 66, part 1
  EXPECT_EQ(railcom::sendShortAddress( 66, 1 ), 0x95A9 ); // Send short address 66, part 2
  EXPECT_EQ(railcom::sendShortAddress( 66, 2 ), 0xA3AC ); // Send short address 66, wrap 3 to part 1 again
  EXPECT_EQ(railcom::sendShortAddress( 66, 3 ), 0x95A9 );
}

TEST(AddressBroadcasetTest, LongAddressCh1) {
  EXPECT_EQ(railcom::sendLongAddress( 0xc3, 0xEC, 0 ), 0x9CA5 );
  EXPECT_EQ(railcom::sendLongAddress( 0xc3, 0xEC, 1 ), 0x96D8 );
  EXPECT_EQ(railcom::sendLongAddress( 0xc3, 0xEC, 2 ), 0x9CA5 );
  EXPECT_EQ(railcom::sendLongAddress( 0xc3, 0xEC, 3 ), 0x96D8 );
}

#if defined(ARDUINO)
#include <Arduino.h>

void setup()
{
    // should be the same value as for the `test_speed` option in "platformio.ini"
    // default value is test_speed=115200
    Serial.begin(115200);

    ::testing::InitGoogleTest();
    // if you plan to use GMock, replace the line above with
    // ::testing::InitGoogleMock();
}

void loop()
{
  // Run tests
  if (RUN_ALL_TESTS())
  ;

  // sleep for 1 sec
  delay(1000);
}

#else
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // if you plan to use GMock, replace the line above with
    // ::testing::InitGoogleMock(&argc, argv);

    if (RUN_ALL_TESTS()) {

    }
    ;

    // Always return zero-code and allow PlatformIO to parse results
    return 0;
}
#endif