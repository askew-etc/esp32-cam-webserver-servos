/*
 * simpleviewer and streamviewer
 */

const uint8_t index_simple_html[] = R"=====(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title id="title">ESP32-CAM Simplified View</title>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png">
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png">
    <link rel="stylesheet" type="text/css" href="/style.css">
    <style>
      @media (min-width: 800px) and (orientation:landscape) {
        #content {
          display:flex;
          flex-wrap: nowrap;
          flex-direction: column;
          align-items: flex-start;
        }
      }
    </style>
  </head>

  <body>
    <section class="main">
      <div id="logo">
        <label for="nav-toggle-cb" id="nav-toggle" style="float:left;" title="Settings">&#9776;&nbsp;</label>
        <button id="swap-viewer" style="float:left;" title="Swap to full feature viewer">Full</button>
        <button id="get-still" style="float:left;">Get Still</button>
        <button id="toggle-stream" style="float:left;" class="hidden">Start Stream</button>
        <button id="menu-toggle" style="float:left;">MIN</button>
        <div id="wait-settings" style="float:left;" class="loader" title="Waiting for camera settings to load"></div>
      </div>
      <div id="content">
        <div class="hidden" id="sidebar">
          <input type="checkbox" id="nav-toggle-cb">
            <nav id="menu" style="width:24em;">
              <div class="input-group hidden" id="lamp-group" title="Flashlight LED.&#013;&#013;Warning:&#013;Built-In lamps can be Very Bright! Avoid looking directly at LED&#013;Can draw a lot of power and may cause visual artifacts, affect WiFi or even brownout the camera on high settings">
                <label for="lamp">Light</label>
                <div class="range-min">Off</div>
                <input type="range" id="lamp" min="0" max="100" value="0" class="action-setting">
                <div class="range-max">Full&#9888;</div>
              </div>
              <div class="input-group" id="servo_pos-group">
                <label>Servo Saved Positions</label>
                <button id="servo_pos1" value="90-90" class="default-action servo-save">P1</button>
                <button id="servo_pos2" value="90-90" class="default-action servo-save">P2</button>
                <button id="servo_pos3" value="90-90" class="default-action servo-save">P3</button>
                <button id="servo_pos4" value="90-90" class="default-action servo-save">P4</button>
              </div>
              <div class="input-group" id="servo_move-group">
                <label>Servo Movement</label>
                <button id="servo_left">&larr;</button>
                <button id="servo_right">&rarr;</button>
                <button id="servo_up">&uarr;</button>
                <button id="servo_down">&darr;</button>
              </div>
              <div class="input-group" id="servo_horz-group">
                <label for="servo_horz">Servo Horizontal</label>
                <div class="range-min">0</div>
                <input type="range" id="servo_horz" min="0" max="180" value="90" class="default-action">
                <div class="range-max">180</div>
              </div>
              <div class="input-group" id="servo_vert-group">
                <label for="servo_vert">Servo Vertical</label>
                <div class="range-min">10</div>
                <input type="range" id="servo_vert" min="10" max="170" value="90" class="default-action">
                <div class="range-max">170</div>
              </div>
              <div class="input-group" id="servo_savpos-group">
                <label>Save Servo Position</label>
                <button id="servo_savpos1">S1</button>
                <button id="servo_savpos2">S2</button>
                <button id="servo_savpos3">S3</button>
                <button id="servo_savpos4">S4</button>
              </div>
              <div class="input-group" id="servo_rev_vert-group" title="Enable reversed movement of vertical servo">
                <label for="reverse_vert">Reverse Vertical Servo</label>
                <div class="switch">
                  <input id="reverse_vert" type="checkbox" class="default-action">
                  <label class="slider" for="reverse_vert"></label>
                </div>
              </div>
              <div class="input-group" id="servo_rev_horz-group" title="Enable reversed movement of horizontal servo">
                <label for="reverse_horz">Reverse Horizontal Servo</label>
                <div class="switch">
                  <input id="reverse_horz" type="checkbox" class="default-action">
                  <label class="slider" for="reverse_horz"></label>
                </div>
              </div>
              <div class="input-group" id="framesize-group">
                <label for="framesize">Resolution</label>
                <select id="framesize" class="action-setting">
                  <option value="13">UXGA (1600x1200)</option>
                  <option value="12">SXGA (1280x1024)</option>
                  <option value="11">HD (1280x720)</option>
                  <option value="10">XGA (1024x768)</option>
                  <option value="9">SVGA (800x600)</option>
                  <option value="8">VGA (640x480)</option>
                  <option value="7">HVGA (480x320)</option>
                  <option value="6">CIF (400x296)</option>
                  <option value="5">QVGA (320x240)</option>
                  <option value="3">HQVGA (240x176)</option>
                  <option value="1">QQVGA (160x120)</option>
                  <option value="0">THUMB (96x96)</option>
                </select>
              </div>
              <!-- Hide the next entries, they are present in the body so that we
                  can pass settings to/from them for use in the scripting, not for user setting -->
              <div id="rotate" class="action-setting hidden"></div>
              <div id="cam_name" class="action-setting hidden"></div>
              <div id="stream_url" class="action-setting hidden"></div>
            </nav>
        </div>
        <figure>
          <div id="stream-container" class="image-container hidden">
            <div class="close close-rot-none" id="close-stream">×</div>
            <img id="stream" src="">
          </div>
        </figure>
      </div>
    </section>
  </body>

  <script>
  document.addEventListener('DOMContentLoaded', function (event) {
    var baseHost = document.location.origin;
    var streamURL = 'Undefined';
    var prevShownElems = [];
    var alwaysShownElems = ['lamp-group', 'servo_pos-group', 'servo_move-group', 'servo_horz-group', 'servo_vert-group', 'framesize-group'];

    const settings = document.getElementById('sidebar')
    const waitSettings = document.getElementById('wait-settings')
    const lampGroup = document.getElementById('lamp-group')
    const servoVert = document.getElementById('servo_vert')
    const servoHorz = document.getElementById('servo_horz')
    const reverseVert = document.getElementById('reverse_vert')
    const reverseHorz = document.getElementById('reverse_horz')
    const servoLeftButton = document.getElementById('servo_left')
    const servoRightButton = document.getElementById('servo_right')
    const servoUpButton = document.getElementById('servo_up')
    const servoDownButton = document.getElementById('servo_down')
    const servoPos1Button = document.getElementById('servo_pos1')
    const servoPos2Button = document.getElementById('servo_pos2')
    const servoPos3Button = document.getElementById('servo_pos3')
    const servoPos4Button = document.getElementById('servo_pos4')
    const servoSave1Button = document.getElementById('servo_savpos1')
    const servoSave2Button = document.getElementById('servo_savpos2')
    const servoSave3Button = document.getElementById('servo_savpos3')
    const servoSave4Button = document.getElementById('servo_savpos4')
    const rotate = document.getElementById('rotate')
    const view = document.getElementById('stream')
    const viewContainer = document.getElementById('stream-container')
    const stillButton = document.getElementById('get-still')
    const streamButton = document.getElementById('toggle-stream')
    const closeButton = document.getElementById('close-stream')
    const swapButton = document.getElementById('swap-viewer')
    const menuToggleButton = document.getElementById('menu-toggle')

    const hide = el => {
      el.classList.add('hidden')
    }
    const show = el => {
      el.classList.remove('hidden')
    }

    const disable = el => {
      el.classList.add('disabled')
      el.disabled = true
    }

    const enable = el => {
      el.classList.remove('disabled')
      el.disabled = false
    }

    const updateValue = (el, value, updateRemote) => {
      updateRemote = updateRemote == null ? true : updateRemote
      let initialValue
      if (el.type === 'checkbox') {
        initialValue = el.checked
        value = !!value
        el.checked = value
      } else {
        initialValue = el.value
        el.value = value
      }

      if (updateRemote && initialValue !== value) {
        updateConfig(el);
      } else if(!updateRemote){
        if(el.id === "lamp"){
          if (value == -1) {
            hide(lampGroup)
          } else {
            show(lampGroup)
          }
        } else if (el.id === "servo_vert") {
          servoVert.value = value;
        } else if (el.id === "servo_horz") {
          servoHorz.value = value;
        } else if (el.id === "reverse_vert") {
          //reverseVert.value = value;
          if (value) {
            servoVert.style.direction = 'rtl';
            servoUpButton.textContent = '↓';
            servoDownButton.textContent = '↑';
          } else {
            servoVert.style.direction = 'ltr';
            servoUpButton.textContent = '↑';
            servoDownButton.textContent = '↓';
          }
        } else if (el.id === "reverse_horz") {
          //reverseHorz.value = value;
          if (value) {
            servoHorz.style.direction = 'rtl';
            servoLeftButton.textContent = '→';
            servoRightButton.textContent = '←';
          } else {
            servoHorz.style.direction = 'ltr';
            servoLeftButton.textContent = '←';
            servoRightButton.textContent = '→';
          }
        } else if(el.id === "cam_name"){
          window.document.title = value;
          console.log('Name set to: ' + value);
        } else if(el.id === "code_ver"){
          console.log('Firmware Build: ' + value);
        } else if(el.id === "rotate"){
          rotate.value = value;
          applyRotation();
        } else if(el.id === "stream_url"){
          streamURL = value;
          streamButton.setAttribute("title", `Start the stream :: {streamURL}`);
          console.log('Stream URL set to:' + value);
        }
      }
    }

    var rangeUpdateScheduled = false
    var latestRangeConfig

    function updateRangeConfig (el) {
      latestRangeConfig = el
      if (!rangeUpdateScheduled) {
        rangeUpdateScheduled = true;
        setTimeout(function(){
          rangeUpdateScheduled = false
          updateConfig(latestRangeConfig)
        }, 150);
      }
    }

    function updateConfig (el) {
      let value
      switch (el.type) {
        case 'checkbox':
          value = el.checked ? 1 : 0
          break
        case 'range':
        case 'select-one':
          value = el.value
          break
        case 'button':
        case 'submit':
          if (el.classList.contains('servo-save')) {
            value = el.value
          } else {
            value = '1'
          }
          break
        default:
          return
      }

      const query = `${baseHost}/control?var=${el.id}&val=${value}`

      fetch(query)
        .then(response => {
          console.log(`request to ${query} finished, status: ${response.status}`)
        })
    }

    document
      .querySelectorAll('.close')
      .forEach(el => {
        el.onclick = () => {
          hide(el.parentNode)
        }
      })

    // read initial values
    fetch(`${baseHost}/status`)
      .then(function (response) {
        return response.json()
      })
      .then(function (state) {
        document
          .querySelectorAll('.action-setting')
          .forEach(el => {
            updateValue(el, state[el.id], false)
          })
        hide(waitSettings);
        show(settings);
        show(streamButton);
        startStream();
        // move servos to retrieved values
        updateConfig(servoHorz);
        updateConfig(servoVert);
      })

    // Put some helpful text on the 'Still' button
    stillButton.setAttribute("title", `Capture a still image :: ${baseHost}/capture`);

    const stopStream = () => {
      window.stop();
      streamButton.innerHTML = 'Start Stream';
          streamButton.setAttribute("title", `Start the stream :: ${streamURL}`);
      hide(viewContainer);
    }

    const startStream = () => {
      view.src = streamURL;
      view.scrollIntoView(false);
      streamButton.innerHTML = 'Stop Stream';
      streamButton.setAttribute("title", `Stop the stream`);
      show(viewContainer);
    }

    const applyRotation = () => {
      rot = rotate.value;
      if (rot == -90) {
        viewContainer.style.transform = `rotate(-90deg)  translate(-100%)`;
        closeButton.classList.remove('close-rot-none');
        closeButton.classList.remove('close-rot-right');
        closeButton.classList.add('close-rot-left');
      } else if (rot == 90) {
        viewContainer.style.transform = `rotate(90deg) translate(0, -100%)`;
        closeButton.classList.remove('close-rot-left');
        closeButton.classList.remove('close-rot-none');
        closeButton.classList.add('close-rot-right');
      } else {
        viewContainer.style.transform = `rotate(0deg)`;
        closeButton.classList.remove('close-rot-left');
        closeButton.classList.remove('close-rot-right');
        closeButton.classList.add('close-rot-none');
      }
       console.log('Rotation ' + rot + ' applied');
    }

    const hideUnusedElems = () => {
      const menuDivs = document.getElementById('menu');
      for (const x of menuDivs.children) {
        if (!(alwaysShownElems.includes(x.id)) && !(x.classList.contains('hidden'))) {
          prevShownElems.push(x.id);
          x.classList.add('hidden');
        }
      }
      document.getElementById('menu-toggle').textContent = "MAX";
    }

    const showUnusedElems = () => {
      let elem;
      for (const x of prevShownElems) {
        elem = document.getElementById(x);
        elem.classList.remove('hidden');
      }
      prevShownElems = [];
      document.getElementById('menu-toggle').textContent = "MIN"
    }

    // Attach actions to controls

    stillButton.onclick = () => {
      stopStream();
      view.src = `${baseHost}/capture?_cb=${Date.now()}`;
      view.scrollIntoView(false);
      show(viewContainer);
    }

    closeButton.onclick = () => {
      stopStream();
      hide(viewContainer);
    }

    streamButton.onclick = () => {
      const streamEnabled = streamButton.innerHTML === 'Stop Stream'
      if (streamEnabled) {
        stopStream();
      } else {
        startStream();
      }
    }

    menuToggleButton.onclick = () => {
      if (menuToggleButton.textContent == 'MIN') {
        hideUnusedElems();
      } else if (menuToggleButton.textContent == 'MAX') {
        showUnusedElems();
      }
    }

    // Attach default on change action
    document
      .querySelectorAll('.action-setting')
      .forEach(el => {
        el.onchange = () => updateConfig(el)
      })

    // Update range sliders as they are being moved
    document
      .querySelectorAll('input[type="range"]')
      .forEach(el => {
        el.oninput = () => updateRangeConfig(el)
      })

    // Custom actions
    // Detection and framesize
    rotate.onchange = () => {
      applyRotation();
      updateConfig(rotate);
    }
    
    servoVert.onchange = () => {
      updateConfig(servoVert);
    }

    servoHorz.onchange = () => {
      updateConfig(servoHorz);
    }

    reverseVert.onchange = () => {
      updateConfig(reverseVert);
      if (reverseVert.checked) {
        servoVert.style.direction = 'rtl';
        servoUpButton.textContent = '↓';
        servoDownButton.textContent = '↑';
      } else {
        servoVert.style.direction = 'ltr';
        servoUpButton.textContent = '↑';
        servoDownButton.textContent = '↓';
      }
    }

    reverseHorz.onchange = () => {
      updateConfig(reverseHorz);
      if (reverseHorz.checked) {
        servoHorz.style.direction = 'rtl';
        servoLeftButton.textContent = '→';
        servoRightButton.textContent = '←';
      } else {
        servoHorz.style.direction = 'ltr';
        servoLeftButton.textContent = '←';
        servoRightButton.textContent = '→';
      }
    }

    servoLeftButton.onclick = () => {
      if (parseInt(servoHorz.value) >= 10) {
        servoHorz.value -= 10;
        updateConfig(servoHorz);
      }
    }

    servoRightButton.onclick = () => {
      if (parseInt(servoHorz.value) <= 170) {
        servoHorz.value = servoHorz.value * 1 + 10;
        updateConfig(servoHorz);
      }
    }

    servoDownButton.onclick = () => {
      if (parseInt(servoVert.value) >= 20) {
        servoVert.value -= 10;
        updateConfig(servoVert);
      }      
    }

    servoUpButton.onclick = () => {
      if (parseInt(servoVert.value) <= 160) {
        servoVert.value = servoVert.value * 1 + 10;
        updateConfig(servoVert);
      }
    }

    servoSave1Button.onclick = () => {
      servoPos1Button.value = servoHorz.value + '-' + servoVert.value;
      updateConfig(servoPos1Button);
    }

    servoSave2Button.onclick = () => {
      servoPos2Button.value = servoHorz.value + '-' + servoVert.value;
      updateConfig(servoPos2Button);
    }

    servoSave3Button.onclick = () => {
      servoPos3Button.value = servoHorz.value + '-' + servoVert.value;
      updateConfig(servoPos3Button);
    }

    servoSave4Button.onclick = () => {
      servoPos4Button.value = servoHorz.value + '-' + servoVert.value;
      updateConfig(servoPos4Button);
    }

    servoPos1Button.onclick = () => {
      servoHorz.value = servoPos1Button.value.split('-')[0];
      servoVert.value = servoPos1Button.value.split('-')[1];
      updateConfig(servoHorz);
      updateConfig(servoVert);
    }

    servoPos2Button.onclick = () => {
      servoHorz.value = servoPos2Button.value.split('-')[0];
      servoVert.value = servoPos2Button.value.split('-')[1];
      updateConfig(servoHorz);
      updateConfig(servoVert);
    }

    servoPos3Button.onclick = () => {
      servoHorz.value = servoPos3Button.value.split('-')[0];
      servoVert.value = servoPos3Button.value.split('-')[1];
      updateConfig(servoHorz);
      updateConfig(servoVert);
    }

    servoPos4Button.onclick = () => {
      servoHorz.value = servoPos4Button.value.split('-')[0];
      servoVert.value = servoPos4Button.value.split('-')[1];
      updateConfig(servoHorz);
      updateConfig(servoVert);
    }
    
    framesize.onchange = () => {
      updateConfig(framesize)
    }

    swapButton.onclick = () => {
      window.open('/?view=full','_self');
    }

  })
  </script>
</html>)=====";

size_t index_simple_html_len = sizeof(index_simple_html)-1;

/* Stream Viewer */

const uint8_t streamviewer_html[] = R"=====(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title id="title">ESP32-CAM StreamViewer</title>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png">
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png">
    <style>
      /* No stylesheet, define all style elements here */
      body {
        font-family: Arial,Helvetica,sans-serif;
        background: #181818;
        color: #EFEFEF;
        font-size: 16px;
        margin: 0px;
        overflow:hidden;
      }

      img {
        object-fit: contain;
        display: block;
        margin: 0px;
        padding: 0px;
        width: 100vw;
        height: 100vh;
      }

      .loader {
        border: 0.5em solid #f3f3f3;
        border-top: 0.5em solid #000000;
        border-radius: 50%;
        width: 1em;
        height: 1em;
        -webkit-animation: spin 2s linear infinite; /* Safari */
        animation: spin 2s linear infinite;
      }

      @-webkit-keyframes spin {   /* Safari */
        0% { -webkit-transform: rotate(0deg); }
        100% { -webkit-transform: rotate(360deg); }
      }

      @keyframes spin {
        0% { transform: rotate(0deg); }
        100% { transform: rotate(360deg); }
      }
    </style>
  </head>

  <body>
    <section class="main">
      <div id="wait-settings" style="float:left;" class="loader" title="Waiting for stream settings to load"></div>
      <div style="display: none;">
        <!-- Hide the next entries, they are present in the body so that we
             can pass settings to/from them for use in the scripting -->
        <div id="rotate" class="action-setting hidden">0</div>
        <div id="cam_name" class="action-setting hidden"></div>
        <div id="stream_url" class="action-setting hidden"></div>
      </div>
      <img id="stream" src="">
    </section>
  </body>

  <script>
  document.addEventListener('DOMContentLoaded', function (event) {
    var baseHost = document.location.origin;
    var streamURL = 'Undefined';

    const rotate = document.getElementById('rotate')
    const stream = document.getElementById('stream')
    const spinner = document.getElementById('wait-settings')

    const updateValue = (el, value, updateRemote) => {
      updateRemote = updateRemote == null ? true : updateRemote
      let initialValue
      if (el.type === 'checkbox') {
        initialValue = el.checked
        value = !!value
        el.checked = value
      } else {
        initialValue = el.value
        el.value = value
      }

      if (updateRemote && initialValue !== value) {
        updateConfig(el);
      } else if(!updateRemote){
        if(el.id === "cam_name"){
          window.document.title = value;
          stream.setAttribute("title", value + "\n(doubleclick for fullscreen)");
          console.log('Name set to: ' + value);
        } else if(el.id === "rotate"){
          rotate.value = value;
          console.log('Rotate recieved: ' + rotate.value);
        } else if(el.id === "stream_url"){
          streamURL = value;
          console.log('Stream URL set to:' + value);
        }
      }
    }

    // read initial values
    fetch(`${baseHost}/info`)
      .then(function (response) {
        return response.json()
      })
      .then(function (state) {
        document
          .querySelectorAll('.action-setting')
          .forEach(el => {
            updateValue(el, state[el.id], false)
          })
        spinner.style.display = `none`;
        applyRotation();
        startStream();
      })

    const startStream = () => {
      stream.src = streamURL;
      stream.style.display = `block`;
    }

    const applyRotation = () => {
      rot = rotate.value;
      if (rot == -90) {
        stream.style.transform = `rotate(-90deg)`;
      } else if (rot == 90) {
        stream.style.transform = `rotate(90deg)`;
      }
      console.log('Rotation ' + rot + ' applied');
    }

    stream.ondblclick = () => {
      if (stream.requestFullscreen) {
        stream.requestFullscreen();
      } else if (stream.mozRequestFullScreen) { /* Firefox */
        stream.mozRequestFullScreen();
      } else if (stream.webkitRequestFullscreen) { /* Chrome, Safari and Opera */
        stream.webkitRequestFullscreen();
      } else if (stream.msRequestFullscreen) { /* IE/Edge */
        stream.msRequestFullscreen();
      }
    }
  })
  </script>
</html>)=====";

size_t streamviewer_html_len = sizeof(streamviewer_html)-1;

/* Captive Portal page
   we replace the <> delimited strings with correct values as it is served */

const std::string portal_html = R"=====(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title id="title"><CAMNAME> - portal</title>
    <link rel="icon" type="image/png" sizes="32x32" href="<APPURL>favicon-32x32.png">
    <link rel="icon" type="image/png" sizes="16x16" href="<APPURL>favicon-16x16.png">
    <link rel="stylesheet" type="text/css" href="<APPURL>style.css">
  </head>
  <body style="text-align: center;">
    <img src="<APPURL>logo.svg" style="position: relative; float: right;">
    <h1><CAMNAME> - access portal</h1>
    <div class="input-group" style="margin: auto; width: max-content;">
      <a href="<APPURL>?view=simple" title="Click here for a simple view with minimum control" style="text-decoration: none;" target="_blank">
      <button>Simple Viewer</button></a>
      <a href="<APPURL>?view=full" title="Click here for the main camera page with full controls" style="text-decoration: none;" target="_blank">
      <button>Full Viewer</button></a>
      <a href="<STREAMURL>view" title="Click here for the dedicated stream viewer" style="text-decoration: none;" target="_blank">
      <button>Stream Viewer</button></a>
    </div>
    <hr>
    <a href="<APPURL>dump" title="Information dump page" target="_blank">Camera Details</a><br>
  </body>
</html>)=====";

/* Error page
   we replace the <> delimited strings with correct values as it is served */

const std::string error_html = R"=====(<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title id="title"><CAMNAME> - Error</title>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png">
    <link rel="ico\" type="image/png" sizes="16x16" href="/favicon-16x16.png">
    <link rel="stylesheet" type="text/css" href="<APPURL>style.css">
  </head>
  <body style="text-align: center;">
    <img src="<APPURL>logo.svg" style="position: relative; float: right;">
    <h1><CAMNAME></h1>
    <ERRORTEXT>
  </body>
  <script>
    setTimeout(function(){
      location.replace(document.URL);
    }, 60000);
  </script>
</html>)=====";
