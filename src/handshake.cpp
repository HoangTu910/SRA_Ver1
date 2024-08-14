// #include "handshake.h"

// uint8_t receivePacket(uint8_t expectedData, uint32_t timeout){
//     Serial1.begin(115200, SERIAL_8N1, 16, 17);
//     unsigned long startTime = millis();
//     while (millis() - startTime < timeout) {
//         if (Serial1.available() > 0) {
//             uint8_t receivedData = Serial.read();
//             if (receivedData == expectedData) {
//                 return true; 
//             }
//         }
//     }
//     return false; 
// }

// uint8_t receiveThreeWayHandshake(){
//     if (!receivePacket(COMMAND_SYN, TIME_OUT)) {
//         Serial.println("Failed to received (Handshake step 1)");
//         return 0; 
//     }
//     Serial.print("Received packet [");
//     Serial.print(COMMAND_SYN);
//     Serial.println("]");

//     Serial.write(COMMAND_SYN_ACK); //send acknowledment
//     Serial.print("Send accept packet [");
//     Serial.print(COMMAND_SYN_ACK);
//     Serial.println("]");

//     if (!receivePacket(COMMAND_ACK, TIME_OUT)) {
//         Serial.println("Failed to received (Handshake step 3)");
//         return 0; 
//     }

//     Serial.println("Three way handshake success!");
//     return 1; 
// }