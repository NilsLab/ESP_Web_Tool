const char upload_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0">
    <title>ESP Web Tool</title>
    <!--<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>-->  
   <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      .content {padding: 8px;margin:0px 0px;}

      .card { background-color: white; 
        box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
        border-radius: 10px;
        width:60%;
        margin-left: auto;
        margin-right: auto;
      }
      
      input{
        background:#f1f1f1;
        border:0;
        width:90%;
      }
      
      .button {
        border: none;
        color: white;      
        margin: 4px 2px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        cursor: pointer;
        font-weight: bold;
        opacity: 0.8;
        
      }
      .button:hover {
        opacity: 1;
      }
      
      .button::after{
        clear:both;
      }
      
      .button2 {background-color: #0db5f2;width: 95%;}
      
      #file-input,input{width:95%;height:34px;border-radius:4px;margin:10px auto;font-size:15px}
      input{background:#f1f1f1;border:0;padding:0 15px}
      #file-input{padding:0;border:1px solid #ddd;line-height:34px;text-align:center;display:block;cursor:pointer}
     
      #prgbar{background-color:#f1f1f1;border-radius:10px;}
      #bar{background-color:#FF0500;width:0%;height:10px;}

      
  </style>
  </head>
<body style="margin:0px 0px;">
  <div class="content" >
    <div class="card" >   
     <br>
        <h2>ESP Web Tool</h2>
        <h5>Upload index.html</h5>
        
        <form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>
        <div align="center" style="padding: 5px 5px;">
          <input type='file' name='update' id='file' onchange='sub(this)' style="display:none"> </input>
          <label id='file-input' for='file' style="padding: 3px 0%;">Choose Firmware file</label>
      
          <input type='submit' class="button button2" value='Upload'>
        </div>
        
        <div id='prg'>
        
        </div>
        
        <div align="center" id='prgbar' >
          <div id='bar'></div>
        </div>        
      </form>
      <br>
      <a href="./">Home</a> 
      <a href="./terminal">Web Terminal</a> 
      <br>
      <br>
    </div>
  </div>

<script language="javascript" type="text/javascript">

//var url = "ws://192.168.1.2:1337/";
var url = 'ws://'+location.hostname+':1337/'
var textinput;
var canvas;
var context;
var dis;
// This is called when the page finishes loading
function init() {

   // textinput = document.getElementById("message");
    // Connect to WebSocket server
    //wsConnect(url);
}


function sub(obj){
  var fileName = obj.value.split('\\\\');
  document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];
};


// Call the init function as soon as the page loads
window.addEventListener("load", init, false);

</script>
</body>
</html>)rawliteral";
