  var engineReq = new XMLHttpRequest();
  var gameReq = new XMLHttpRequest();

  function getParameterByName(name) {
    name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
    var regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
    results = regex.exec(location.search);
    return results === null ? null : decodeURIComponent(results[1].replace(/\+/g, " "));
  }

  var loveFile = getParameterByName('love') || 'game.love';

  function loadError(msg) {
    engine_progress2.style.display="none";
    engine_progress3.innerHTML = "<img style=\"top: 3px; position: relative\" src=\"" + errordata + "\"> " + msg;
    engine_progress3.style.color = "#692828";
    engine_progress3.style.height = "15px";
  }

  function loadGame() {
    Module['addRunDependency']("zip game.love");
    zip.useWebWorkers=false;

    Module['FS_createPath']("/", "love", 1, 1)
    zip.createReader(new zip.BlobReader(gameReq.response), function(reader) {
      reader.getEntries(function(entries) {
        var i;
        for(i=0; i < entries.length; ++i) {
          if(entries[i].directory) {
            Module['FS_createPath']("/love/", entries[i].filename, 1, 1);
          } else {
            Module['addRunDependency']("fp " + entries[i].filename);
            entries[i].getData(
              new zip.ArrayBufferWriter(), 
              (function(entry) {
                return function(data) {
                  Module['FS_createPreloadedFile']("/love/" + entry.filename, null, new Uint8Array(data), true, true,
                    function() { // ondone
                      Module['removeRunDependency']("fp " + entry.filename);
                    },
                    function() { // onerror
                      loadError("Could not create preload file \"" + entry.filename + "\"");
                    },
                    false,
                    false);
                }
              })(entries[i]), 
              
              function() {
                loadError("Could not read ZIP contents");
              });
          }
        }
        Module['removeRunDependency']('zip game.love');
      });
    }, function(error) {
      loadError("Could not read ZIP contents:" + error);
      
    });
  }

  var Module = {
    preRun: [loadGame],
    postRun: [],
    print: (function() {
      return function(text) {
        text = Array.prototype.slice.call(arguments).join(' ');
        console.log(text);
      };
    })(),
    printErr: function(text) {
      text = Array.prototype.slice.call(arguments).join(' ');
      console.error(text);
    },
    TOTAL_MEMORY:67108864
  };

  if(motor) {
    if(motor.mem) {
      Module.TOTAL_MEMORY = motor.mem;
    }
  }

  var enginesize, gamesize;
  var engineloaded = 0, gameloaded = 0
  function updateProgress() {
    if(enginesize == 1 || gamesize == 1) {
      var kb = Math.floor((engineloaded + gameloaded) / 1024);
      engine_progress3.innerHTML = kb + "KiB loaded<br>(sorry, total size unknown)";
    } else {
      var ratio = Math.floor(100 *(engineloaded + gameloaded) / (enginesize + gamesize))
      engine_progress.style.width = (4*ratio) + "px";
      engine_progress3.innerHTML = ratio + "% loaded";
    }
  }

  var layer = document.getElementById('motor_output');
  var canvas;
  var engine_progress, engine_progress2, engine_progress3
  if(!layer) {
    document.body.innerHTML = "<img src=\"" + logodata + "\"><br>Could not find motor_output layer. Please add &lt;div id=\"motor_output\"&gt;&lt;/div&gt; in the appropriate place in your HTML";

  } else {
    var layer2 = document.createElement('div');
    var layer3 = document.createElement('div');
    var layer4 = document.createElement('div');

    layer4.style.height="100%";
    layer4.style.display="table";
    layer4.style.width="100%";

    layer3.style.display="table-cell";
    layer3.style.verticalAlign="middle";
    layer4.appendChild(layer3);

    layer2.style.marginLeft = "auto";
    layer2.style.marginRight = "auto";
    //layer2.style.width="400px";
    layer3.appendChild(layer2);


    layer.appendChild(layer4);

    var img = document.createElement('img');
    img.src=logodata;
    layer2.appendChild(img);
    var green = "#BF3D3D";
    var red = "#eee";
    engine_progress = document.createElement('div');
    engine_progress.style.backgroundColor = green;
    engine_progress.style.width="0px";
    //engine_progress.style.paddingLeft="5px";
    engine_progress.style.textAlign="left";
    engine_progress.style.overflow="hidden";
    engine_progress.style.whiteSpace ="nowrap";
    engine_progress.style.color = "#692828";
    engine_progress.style.fontFamily = "arial";
    engine_progress.style.fontSize = "12px";
    engine_progress.style.height = "15px";
    engine_progress.style.position = "relative";

    engine_progress2 = document.createElement('div');
    engine_progress2.style.width="400px";
    engine_progress2.style.height="15px";
    engine_progress2.style.marginLeft="auto";
    engine_progress2.style.marginRight="auto";
    engine_progress2.style.marginTop = "2px";
    engine_progress2.style.backgroundColor = red;
    engine_progress2.style.overflow="hidden";
    layer2.appendChild(engine_progress2);
    engine_progress2.appendChild(engine_progress);

    engine_progress3 = document.createElement('div');
    engine_progress3.style.align="center";
    engine_progress3.style.marginTop="5px";
    engine_progress3.style.fontSize="12px";
    engine_progress3.style.color = "#888";
    layer2.appendChild(engine_progress3);

    function bootEngine() {
      if(engineReq.readyState == 4 && gameReq.readyState == 4) {
        if(gameReq.status == 200 && engineReq.status == 200) {
          engine_progress3.innerHTML="Unpacking Game...";
          while(layer2.firstChild) {
            layer2.removeChild(layer2.firstChild);
          }
          canvas = document.createElement("canvas");
          canvas.addEventListener("contextmenu", function(event) {event.preventDefault();}, false);
          canvas.id="motor_canvas";
          Module.canvas = canvas;
          canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
          layer2.appendChild(canvas);
          //eval.call(window, engineReq.response);
          
          var scriptTag = document.createElement("script");
          scriptTag.type = "text/javascript";
          scriptTag.src = "motor2d.js";
          document.body.appendChild(scriptTag);
          
        } else if(engineReq.status != 200) {
          loadError("Could not load engine: " + engineReq.statusText);
        } else if(gameReq.status != 200) {
          loadError("Could not load game data: " + gameReq.statusText);
        }
      }
    }



    function loadEngine() {
      if(gamesize && enginesize) {

        engineReq.open('GET', 'motor2d.js', true);
        engineReq.onload = function() {
          console.log("engine loaded");
          bootEngine();
        };
        engineReq.onprogress = function(a) {
          engineloaded = a.loaded;
          updateProgress();
        };
        engineReq.onerror = function(a) {
          loadError("Loading of engine failed");
        };

        engineReq.send();
        gameReq.open('GET', loveFile, true);
        gameReq.onload = function() {
          console.log("game loaded");
          bootEngine();
        };
        gameReq.onprogress = function(a) {
          gameloaded = a.loaded;
          updateProgress();
        };
        gameReq.responseType = "blob";
        gameReq.onprogress = function(a) {
          gameloaded = a.loaded;
          updateProgress();
        };
        gameReq.send();

        // Animate if file size is unknown
        if(gamesize == 1 || enginesize == 1) {
          var l = 0;
          function animateLoadbar() {
            l = (l + 10) % 450;
            engine_progress.style.left = (l-50) + "px";
            if(engineReq.readyState != 4 || gameReq.readyState != 4) {
              setTimeout(animateLoadbar, 50);
            }
          }
          setTimeout(animateLoadbar, 50);
          engine_progress.style.width="50px";
          engine_progress.style.left = "-50px";
        }
      }
    }

    engineReq.open('HEAD', 'motor2d.js', true);
    engineReq.onload = function() {
      enginesize = parseInt(engineReq.getResponseHeader('Content-Length') || "1");
      loadEngine();
    };
    engineReq.onerror = function() {
      enginesize = 1;
      loadEngine();
    };
    engineReq.send();

    gameReq.open('HEAD', loveFile, true);
    gameReq.onload = function() {
      gamesize = parseInt(gameReq.getResponseHeader('Content-Length') || "1");
      loadEngine();
    };
    gameReq.onerror = function() {
      enginesize = 1;
      loadEngine();
    };
    gameReq.send();
  }


