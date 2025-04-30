#include <stdio.h>
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE HTML>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <title>ESP WebSocket Server</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <style>
        html, body {
          margin: 0;
          font-family: Arial, Helvetica, sans-serif;
          text-align: center;
          background-color: #f0f2f5;
        }
        .topnav {
          background-color: #143642;
          padding: 20px 0;
        }
        h1 {
          margin: 0;
          font-size: 2rem;
          color: #fff;
        }
      </style>
    </head>
    <body>
      <div class="topnav">
        <h1>ESP WebSocket Server</h1>
      </div>  
    </body>
    </html>
    )rawliteral";
  