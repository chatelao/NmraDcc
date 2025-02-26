
#include <ArduinoFake.h>
#include <EEPROMFake.h>

using namespace fakeit;

#include "NmraDcc.h"
#include "NmraDcc.cpp"
#include <gtest/gtest.h>

unsigned long addDcc(unsigned long time, unsigned long delay) { 

  When(Method(ArduinoFake(), digitalRead)).Return(HIGH);
  When(Method(ArduinoFake(), micros)).Return(time + delay);
  ExternalInterruptHandler();

  When(Method(ArduinoFake(), digitalRead)).Return(LOW);
  When(Method(ArduinoFake(), micros)).Return(time + 2 * delay);
  ExternalInterruptHandler();

  return time + 2 * delay;

}

unsigned long addDccOne(unsigned long time) {
  return addDcc( time, 58 );
}

unsigned long addDccZero(unsigned long time) {
  return addDcc( time, 100 );
}

TEST(TimeTestTest, MinTest) {

  When(Method(ArduinoFake(EEPROM), read)).AlwaysReturn(0xFF);
  When(Method(ArduinoFake(EEPROM), write)).AlwaysReturn();

  NmraDcc  Dcc ;

  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), attachInterrupt)).AlwaysReturn();

  Dcc.pin( 2, 0 );
  Dcc.init( MAN_ID_DIY, 10, FLAGS_MY_ADDRESS_ONLY, 0 );

  unsigned long time = 0;

  //
  // 17 Bits preamble
  //
  EXPECT_EQ(DccRx.State, WAIT_PREAMBLE);
  for(int i = 0; i < 19; i++) {
    time = addDccOne( time );
  }

  //
  // Package-Start bit and first package
  //
  EXPECT_EQ(DccRx.State, WAIT_START_BIT);
  time = addDccZero( time );

  EXPECT_EQ(DccRx.State, WAIT_DATA);
  for(int i = 0; i < 4; i++) {
    time = addDccZero( time );
    time = addDccOne( time );
  }

  //
  // Data-Start bit and second package
  //
  EXPECT_EQ(DccRx.State, WAIT_END_BIT);
  time = addDccZero( time );

  EXPECT_EQ(DccRx.State, WAIT_DATA);
  for(int i = 0; i < 4; i++) {
    time = addDccOne( time );
    time = addDccZero( time );
  }
  //
  // Data-Start bit and XOR checksum
  //
  EXPECT_EQ(DccRx.State, WAIT_END_BIT);
  time = addDccZero( time );

  EXPECT_EQ(DccRx.State, WAIT_DATA);
  for(int i = 0; i < 8; i++) {
    time = addDccOne( time );
    EXPECT_EQ(DccRx.BitCount, i+1);
  }
  
  //
  // Data-Start bit and second package
  //
  EXPECT_EQ(DccRx.State, WAIT_END_BIT);
  time = addDccOne( time );
  
  EXPECT_EQ(DccRx.State, WAIT_PREAMBLE);

  Verify(Method(ArduinoFake(), micros)).Exactly(94);
  Verify(Method(ArduinoFake(), digitalRead)).Exactly(50);

}