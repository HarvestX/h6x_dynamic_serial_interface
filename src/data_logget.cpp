#include <Arduino.h>
#include <M5Core2.h>
#include <SPIFFS.h>
#include <SD.h>
#include "data_logger.hpp"

bool initFileSystem(const char* filename) {
    if (!SPIFFS.begin(true)) {
        M5.Lcd.println("SPIFFS Mount Failed");
        return false;
    }

    if (SPIFFS.exists(filename)) SPIFFS.remove(filename);
    if (SD.exists(filename)) SD.remove(filename);

    File file = SPIFFS.open(filename, FILE_APPEND);
    File file_sd = SD.open(filename, FILE_APPEND);

    if (!file || !file_sd) {
        M5.Lcd.println("File Open Error");
        return false;
    }

    String header = "time,accX,accY,accZ,gyroX,gyroY,gyroZ,pitch,roll,yaw,temp";
    file.println(header);
    file_sd.println(header);
    file.close();
    file_sd.close();
    return true;
}

void writeDataBuffered(const std::vector<String>& buffer, const char* filename) {
    File file = SPIFFS.open(filename, FILE_APPEND);
    File file_sd = SD.open(filename, FILE_APPEND);
    if (file && file_sd) {
        for (const auto& line : buffer) {
            file.println(line);
            file_sd.println(line);
        }
        file.close();
        file_sd.close();
    }
}
