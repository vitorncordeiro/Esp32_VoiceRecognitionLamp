# Aladin

![Aladin](https://media.discordapp.net/attachments/662375257591513088/1436133799879774259/imagem_2025-11-06_202136005-removebg-preview.png?ex=690e7f53\&is=690d2dd3\&hm=d73dd82f23c506954707610a3d3c220d6c6cff4e363ab179a7005aa09bac49ba&=\&format=webp\&quality=lossless)

**Aladin** is a smart home automation system built with an **ESP32**, a **relay module**, and **Firebase Realtime Database**.
It hosts a secure local web interface that allows users to control a lamp using **voice commands** (powered by Google’s Gemini AI).

All Firebase operations—such as logging lamp states, timestamps, energy consumption, and estimated cost—are now handled **via JavaScript** on the web interface, rather than directly by the ESP32.

---

## Overview

The ESP32 acts as a local HTTPS server and hardware controller.
When accessed through a browser, it serves a webpage containing JavaScript logic that:

* Communicates with **Firebase Realtime Database** using the Firebase Web SDK.
* Sends user commands (voice or manual) to the ESP32 via HTTP requests.
* Logs all state changes and energy metrics directly to Firebase.

This structure separates responsibilities:

* **ESP32 (C++)** → Handles hardware control (relay, timing, and HTTPS server).
* **JavaScript (frontend)** → Manages Firebase logging and AI-driven voice commands.

---

## Features

* Voice-controlled lamp switching using **Gemini AI**
* Local HTTPS web interface served by the **ESP32**
* Firebase integration handled by **JavaScript**
* Real-time logging of energy consumption and cost
* Automatic time synchronization via **NTP**

---

## Hardware Requirements

* ESP32 development board
* Single-channel 5V relay module
* 5V lamp or LED load
* USB cable and power supply
* Wi-Fi network access

---

## Software Requirements

* **Arduino IDE** (latest version)
* **ESP32 board package** (Espressif Systems)
* Libraries:

  * `WiFi.h` (included with ESP32 package)
  * `HTTPClient.h` (for REST communication)
  * `NTPClient` by Fabrice Weinberg

---

## Firebase Setup

1. Go to [Firebase Console](https://console.firebase.google.com/).

2. Create a new project and enable **Realtime Database**.

3. Set the database to test mode (for development).

4. Copy your **Database URL** (format: `https://your-project-id.firebaseio.com/`).

5. In the web interface’s JavaScript file, initialize Firebase:

   ```javascript
   const firebaseConfig = {
     apiKey: "your_api_key",
     authDomain: "your_project_id.firebaseapp.com",
     databaseURL: "https://your-project-id.firebaseio.com",
     projectId: "your_project_id",
     storageBucket: "your_project_id.appspot.com",
     messagingSenderId: "your_sender_id",
     appId: "your_app_id"
   };
   firebase.initializeApp(firebaseConfig);
   ```

6. All database writes and reads are now performed through this script.

---

## Gemini AI Setup

1. Generate an API key at [Google AI Studio](https://aistudio.google.com/app/apikey).

2. In the JavaScript file, replace your Gemini key:

   ```javascript
   const GEMINI_API_KEY = "your_gemini_api_key";
   ```

3. The web app sends the recognized voice commands to Gemini for text interpretation and then triggers local actions.

---

## How It Works

1. The ESP32 connects to Wi-Fi and launches an HTTPS server.
2. The user opens the ESP32’s IP (`https://<device_ip>`) in a browser.
3. The web page runs JavaScript that:

   * Uses Gemini AI for voice command interpretation.
   * Updates Firebase with lamp status and statistics.
   * Sends control commands to the ESP32 (turn on/off, toggle mode).
4. The ESP32 activates or deactivates the relay accordingly.

---

## Example Firebase Data

```json
{
  "lamp": {
    "atual": "LIGADA",
    "historico": {
      "-OyzX12abc": {
        "estado": "DESLIGADA",
        "timestamp": "14:32:10",
        "tempo_h": 0.0042,
        "energia_kwh": 0.00003,
        "custo_reais": 0.00002
      }
    }
  }
}
```

---

## Automatic Mode

When automatic mode is active, the ESP32 toggles the lamp every 15 seconds.
The JavaScript frontend logs each state change to Firebase, including power usage and estimated cost, in real time.

---

## Notes

* The ESP32 must have valid **certificate** and **private key** files for HTTPS.
* All Firebase communication is handled in the browser using the official SDK.
* Tested on ESP32 DevKit v1 using Arduino IDE 2.3.2.

---

## License

This project is licensed under the **MIT License**.
You may use, modify, and distribute it freely for educational or research purposes.

