// uint8_t receivePacket(uint8_t expectedData, uint32_t timeout) {
//     unsigned long startTime = millis();
//     while (millis() - startTime < timeout) {
//         if (Serial1.available() > 0) {
//             uint8_t receivedData = Serial1.read();
//             if (receivedData == expectedData) {
//                 Serial.print("Receive Correct Packet: ");
//                 Serial.println(receivedData);
//                 return true; 
//             }
//             else{
//                 Serial.print("Receive Wrong Packet: ");
//                 Serial.println(receivedData);
//             }
//         }
//         delay(10);
//     }
//     return false; 
// }

// void clearUartBuffer() {
//     while (Serial1.available() > 0) {
//         Serial1.read(); // Read and discard data
//     }
// }

// uint8_t receiveThreeWayHandshake() {
//     clearUartBuffer(); 
//     if (!receivePacket(COMMAND_SYN, TIME_OUT)) {
//         Serial.println("Failed to receive (Handshake step 1)");
//         return 0; 
//     }
//     Serial.print("Received packet [");
//     Serial.print(COMMAND_SYN);
//     Serial.println("]");

//     Serial1.write(COMMAND_SYN_ACK); // Send acknowledgment
//     Serial1.flush();
//     Serial.print("Sent acknowledgment [");
//     Serial.print(COMMAND_SYN_ACK);
//     Serial.println("]");

//     clearUartBuffer(); 
//     unsigned long startTime = millis();
//     uint8_t state;
//     Serial.println("Receiving final packet...");
//     while (millis() - startTime < TIME_OUT){
//         state = receivePacket(COMMAND_ACK, TIME_OUT);
//         if(state) break;
//     }

//     if(state) {
//         Serial.println("Three-way handshake success!");
//         return 1;    
//     }
//     else {
//         Serial.println("Unable to receive final packet!");
//         return 0;
//     }
//     return 1; 
// }