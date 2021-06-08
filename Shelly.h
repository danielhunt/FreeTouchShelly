class Shelly {
  public:
    String ip;
    int relayId;
    bool isOn = false;

    Shelly(String _ip, int _relayId, bool _isOn = false) {
      ip = _ip;
      relayId = _relayId;
      isOn = _isOn;

      //    refreshState();
    }

    String getUrl() {
      return "http://" + ip + "/color/" + String(relayId) + "/set";
    }
};

class ShellyHandler {
  private:
    HTTPClient http;
    String protocol = "http";

  public:
    ShellyHandler() {
      http.setReuse(true);
    }

    String getUrl(Shelly shelly) {
      return protocol + "://" + shelly.ip;
    }

    String getRelayUrl(Shelly shelly) {
      return getUrl(shelly) + "/relay/" + String(shelly.relayId);
    }

    String refreshState(Shelly shelly) {
      String url = getRelayUrl(shelly);
      Serial.print("GET: ");
      Serial.println(url);

      http.begin(url);
      int httpResponseCode = http.GET();

      String payload = "{}";
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        bool isOn = doc["ison"];
        Serial.print("Shelly is ");
        if (isOn) {
          Serial.print("on");
        } else {
          Serial.print("off");
        }
        Serial.println("!");
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();

      return payload;
    }

    void toggleState(Shelly shelly) {
      String url = getRelayUrl(shelly);
      bool newState = shelly.isOn ? false : true;
      url += "?turn=";
      url += newState ? "on" : "off";

      const char* urlChars = url.c_str();
      String res = httpGETRequest(urlChars);
      Serial.println("Response:");
      Serial.println(res);

      shelly.isOn = newState;
    }
};

Shelly rgbw2Shelly("192.168.30.161", 0, false);

ShellyHandler shellyHandler;
