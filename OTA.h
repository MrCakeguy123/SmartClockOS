#pragma once

#include "Config.h"
#include "DataManager.h"
#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>


namespace OTA {

static WebServer server(80);
static bool running = false;

static const char *uploadPage = R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartClockOS Update</title>
<style>
body{font-family:sans-serif;background:#1a1a2e;color:#eee;display:flex;
justify-content:center;align-items:center;min-height:100vh;margin:0}
.card{background:#16213e;padding:2em;border-radius:16px;max-width:400px;
text-align:center;box-shadow:0 8px 32px rgba(0,0,0,.4)}
h2{margin:0 0 .5em;color:#e94560}
input[type=file]{margin:1em 0;color:#eee}
button{background:#e94560;color:#fff;border:none;padding:.8em 2em;
border-radius:8px;font-size:1em;cursor:pointer}
button:hover{background:#c73450}
#bar{width:100%;height:20px;background:#0f3460;border-radius:10px;
margin-top:1em;display:none;overflow:hidden}
#fill{width:0%;height:100%;background:#e94560;border-radius:10px;
transition:width .3s}
#msg{margin-top:1em;color:#a8d8ea}
</style></head><body>
<div class="card">
<h2>SmartClockOS</h2>
<p>v)rawliteral" CLOCK_VERSION R"rawliteral(</p>
<form method="POST" action="/update" enctype="multipart/form-data"
id="fm">
<input type="file" name="update" accept=".bin" required><br>
<button type="submit">Upload & Flash</button>
</form>
<div id="bar"><div id="fill"></div></div>
<div id="msg"></div>
</div>
<script>
document.getElementById('fm').addEventListener('submit',function(e){
  e.preventDefault();
  var f=new FormData(this);
  var x=new XMLHttpRequest();
  document.getElementById('bar').style.display='block';
  x.upload.addEventListener('progress',function(ev){
    if(ev.lengthComputable){
      var p=Math.round(ev.loaded*100/ev.total);
      document.getElementById('fill').style.width=p+'%';
      document.getElementById('msg').textContent='Uploading: '+p+'%';
    }
  });
  x.onreadystatechange=function(){
    if(x.readyState==4){
      document.getElementById('msg').textContent=x.status==200?
        'Success! Rebooting...':'Error: '+x.responseText;
    }
  };
  x.open('POST','/update',true);
  x.send(f);
});
</script>
</body></html>
)rawliteral";

static void handleRoot() { server.send(200, "text/html", uploadPage); }

static void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("[OTA] Receiving: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("[OTA] Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

static void handleUpdateEnd() {
  if (Update.hasError()) {
    server.send(500, "text/plain", "Update failed");
  } else {
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();
  }
}

void begin() {
  if (running)
    return;

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdateEnd, handleUpdateUpload);
  server.begin();
  running = true;

  Serial.printf("[OTA] Web update ready at http://%s/\n",
                WiFi.localIP().toString().c_str());
}

void handle() {
  if (running)
    server.handleClient();
}

String getUrl() {
  if (WiFi.status() != WL_CONNECTED)
    return String("Not connected");
  return String("http://") + WiFi.localIP().toString() + "/";
}

} // namespace OTA
