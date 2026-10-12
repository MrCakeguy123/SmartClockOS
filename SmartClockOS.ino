#include "Config.h"
#include "DataManager.h"
#include "Tasks.h"

DataManager Data;
SettingsManager SettingsStore;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.printf("\n=== %s v%s ===\n", CLOCK_NAME, CLOCK_VERSION);
    Serial.printf("Heap: %u, PSRAM: %u\n", ESP.getFreeHeap(), ESP.getFreePsram());
    
    SettingsStore.begin();
    Settings cfg = SettingsStore.load();
    Data.setSettings(cfg);
    
    xTaskCreatePinnedToCore(networkTask, "Network", 12288, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(helperTask, "Helper", 8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(guiTask, "GUI", 32768, nullptr, 2, nullptr, 1);
    
    Serial.println("Setup complete");
}

void loop() {
    vTaskDelete(nullptr);
}
