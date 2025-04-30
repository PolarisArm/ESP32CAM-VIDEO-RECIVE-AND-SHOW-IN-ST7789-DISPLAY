# ESP32CAM-VIDEO-RECIVE-AND-SHOW-IN-ST7789-DISPLAY
Sending video from esp32 cam and showing the video frame in st7789 display connected with another esp32

## For Setup We need to Some Library
```
	bodmer/TFT_eSPI@^2.5.43 - For TFT Lcd
	bodmer/TJpg_Decoder@^1.1.0 - For JPEG DECODING
	links2004/WebSockets@^2.6.1	- For WebSocket Handling
```

As I am using ST7789 display, I had to change settings inside TFT_eSPI lib.

In User_Setup.h file inside TFT_eSPI lib, comment default ILI9341 driver and uncomment ST7789 driver.<br>
[!Image](asset/driverUnc.png)
<br>
Second, Uncomment the width and height for the tft display. For me it was 240X240 <br>
[!Image](asset/wH.png)
<br>
Third we had to uncomment the ESP32 connection for ST7789 Display:<br>
As per lib My connections are

```txt
	TFT_MOSI 23
	TFT_SCLK 18
	TFT_CS   15  // Chip select control pin. Not Used in my Code
	TFT_DC    2  // Data Command control pin
	TFT_RST   4  // Reset pin (could connect to RST pin)
```
[!Image](asset/pin.png)

<br>

# Websocket code:

WiFi Access Point Initialization.
```c++
const char* ssid = "ESP";
const char* password = "12345678";
const char* mdnsAdd = "esp";


WiFi.softAP(ssid,password);
IPAddress softApIp = WiFi.softAPIP();
Serial.println(softApIp); 

// Start mDNS
if (!MDNS.begin(mdnsAdd)) {
Serial.println("Error setting up MDNS responder!");
} else {
Serial.println("mDNS responder started at esp.local");
}
```
Websocket Setup and Initialization: <br>

```c++
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Client disconnected\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connection from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      break;
    }

    case WStype_TEXT:
      Serial.printf("[%u] Received text: %s\n", num, payload);
      break;

    case WStype_BIN:
    {
      Serial.printf("[%u] Received binary data, length: %u, free Heap: %u\n", num, length,ESP.getFreeHeap());
      
      uint16_t  widthI = 0;
      uint16_t heightI = 0;
      
      TJpgDec.getJpgSize(&widthI,&heightI,(const uint8_t*)payload,length);
      TJpgDec.drawJpg(0,0,(const uint8_t*)payload,length);

      unsigned long time = millis()-lastMillis;
      lastMillis = millis();
      sprintf(buf,"Time: %lu ms",time);
      tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
      tft.drawString(buf,80,220);
      break;
    }

  }
}
```
We have four type event handling here <br>

```text

    WStype_DISCONNECTED
    WStype_CONNECTED
    WStype_TEXT
    WStype_BIN
    
```
All of this is boilerplate code. But the WStype_BIN is most crucial as it handles the incoming JPEG image data sent as binary from esp32-cam. Other events are self explanatory.

In setup, we start the http server (Here it is not needed. It is here because I copied it from my another code). <br>

```c++
  server.on("/", HTTP_GET, []() {
    String html = index_html;
    server.send(200, "text/html", html);
  });
  server.begin();
```
It sendss this HTML page when a client is connected to the server vai browser.

```c++
	// Start WebSocket server
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);

	// In the loop
	void loop() {
	server.handleClient();
	webSocket.loop();
}
```
In the set we initialize the websocket server, add an event handler function to handle the event.
<br>

### 🔁 server.handleClient()
- This handles HTTP requests from clients (e.g., when someone opens http://esp.local in a browser). <br>

- It checks if any client has made an HTTP request. <br>

- If someone requests /, it serves the HTML page from index_html. <br>

- This must be called frequently in the main loop to keep the web server responsive. <br>

- This handles HTTP requests from clients (e.g., when someone opens http://esp.local in a browser). <br>

### 🔁 webSocket.loop()

- It checks if any client has made an HTTP request.

- If someone requests /, it serves the HTML page from index_html.

- This must be called frequently in the main loop to keep the web server responsive.

# TFT Setup

```c++
	TFT_eSPI tft = TFT_eSPI();

	// In the setup Side:
	tft.begin();
	tft.setTextColor(0xFFFF,0x0000);
	tft.fillScreen(TFT_GREENYELLOW);
	tft.setFreeFont(FM12);     // Fonts from font.h lib
```

# JPEG decoder setup

```c++
  // Image Scaling
  TJpgDec.setJpgScale(1);

 // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  TJpgDec.setCallback(tft_output);

```
📝 **Explanation**
- **setJpgScale(1)**: Controls how much the JPEG image is scaled down. 1 means no scaling (full resolution).

- **setSwapBytes(true)**: Ensures correct color rendering by swapping the byte order, which is necessary for the TFT_eSPI library (RGB565 format).

- **setCallback(tft_output)**: Registers a custom function (tft_output) that receives each block of pixels from the decoder and pushes it to the screen.

### 🖼️ tft_output() Function Explained 
 This function draws the image on the display.
 ```cpp
 bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
	{
	if (y >= tft.height()) return 0;
	tft.pushImage(x, y, w, h, bitmap);
	return 1;
	}

 ```

📝 **Explanation in Markdown** <br>
Function Purpose: This is a callback function used by TJpg_Decoder to send decoded image blocks to the TFT screen.

**Parameters:**

- x, y: The top-left coordinates where the image block should be drawn.

- w, h: Width and height of the block being drawn.

- bitmap: Pointer to the actual RGB565 pixel data.

**Flow:**

- Checks if the y position is outside the screen height (tft.height()); if so, it stops drawing.

- Uses tft.pushImage() to draw the image block onto the screen.

- Returns true (1) to indicate success.

### Drawing the jpeg on the display:
 
```cpp 
 	uint16_t  widthI = 0;
	uint16_t heightI = 0;
	
	TJpgDec.getJpgSize(&widthI,&heightI,(const uint8_t*)payload,length);
	TJpgDec.drawJpg(0,0,(const uint8_t*)payload,length);

```
Calculating the frame speed. Total frame transfer time from Esp32cam to this Esp32, the decoding and writing time. Average I got 200 ms and spurious 315ms for 240X240 size image.

```cpp
	unsigned long time = millis()-lastMillis;
	lastMillis = millis();
	sprintf(buf,"Time: %lu ms",time);
	tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
	tft.drawString(buf,80,220);
```

# Project Image
[!Image](asset/VideoOfDisplay.gif)

