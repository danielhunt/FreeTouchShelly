
void drawErrorMessage(String message)
{

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 20);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(message);
}

String httpGETRequest(const char* url) {
  HTTPClient http;
  Serial.print("GET: ");
  Serial.println(url);

  // Your IP address with path or Domain name with URL path
  http.begin(url);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

bool loadWifiConfig() {
  if (!FILESYSTEM.exists("/config/wificonfig.json")) {
    Serial.println("[WARNING]: Config file not found!");
    return false;
  }
  fs::File configfile = FILESYSTEM.open("/config/wificonfig.json");

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, configfile);

  strlcpy(wificonfig.ssid, doc["ssid"] | "FAILED", sizeof(wificonfig.ssid));
  strlcpy(wificonfig.password, doc["password"] | "FAILED", sizeof(wificonfig.password));
  strlcpy(wificonfig.hostname, doc["wifihostname"] | "freetouchdeck", sizeof(wificonfig.hostname));

  uint8_t attempts = doc["attempts"] | 10 ;
  wificonfig.attempts = attempts;

  uint16_t attemptdelay = doc["attemptdelay"] | 500 ;
  wificonfig.attemptdelay = attemptdelay;

  configfile.close();

  if (error) {
    Serial.println("[ERROR]: deserializeJson() error");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

bool startWifiOnly() {
  Serial.printf("[INFO]: Connecting to %s\n", wificonfig.ssid);
  if (String(WiFi.SSID()) != String(wificonfig.ssid))
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wificonfig.ssid, wificonfig.password);
    uint8_t attempts = wificonfig.attempts;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (attempts == 0) {
        WiFi.disconnect();
        Serial.println("");
        return false;

      }
      delay(wificonfig.attemptdelay);
      Serial.print(".");
      attempts--;

    }
  }
  Serial.println("");
  return WiFi.status() == WL_CONNECTED;
}
