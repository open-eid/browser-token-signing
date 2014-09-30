<html>
<head>
  <title>test page for plugin</title>
<script type="text/javascript"> 
  function plugin() {
    return document.getElementById('plugin');
  }
 
  function sign() {
    document.getElementById('errorMessage').innerHTML = '';
    document.getElementById('errorCode').innerHTML = '0';
    document.getElementById('signature').innerHTML = '';
    var hash = document.getElementById('hash').value;
    try {
      var id = plugin().getCertificate().id;
      document.getElementById('id').innerHTML = id;
      var signature = plugin().sign(id, hash, { language: 'et' }); // , { language: 'et' }
      var s = "";
      for (var i = 0; i < signature.length; i += 64) {
        s += signature.substr(i, 64) + "<br/>";
      }
      document.getElementById('signature').innerHTML = s;
    }
    catch (e) {
      /* document.getElementById('errorMessage').innerHTML = e.message ? e.message : e; */
      document.getElementById('errorMessage').innerHTML = plugin().errorMessage;
      document.getElementById('errorCode').innerHTML = plugin().errorCode;
    }
  }
 
  function showVersion() {
    document.getElementById('version').innerHTML = plugin().version;
  }
 
  var names = ['id', 'CN', 'issuerCN', 'validFrom', 'validTo'];
 
  function showData() {
    try {
        var cert = plugin().getCertificate();
        for (var i = 0; i < names.length; i++) {
          name = names[i];
          document.getElementById(name).innerHTML = cert[name];
        }
        document.getElementById('PEM').innerHTML = cert.certificateAsPEM;
        var hex = cert.certificateAsHex; 
        var s = "";
        if (hex) for (var i = 0; i < hex.length; i += 64) s += hex.substr(i, 64) + "<br/>";
        document.getElementById('hex').innerHTML = s;
        document.getElementById('errorMessage').innerHTML = plugin().errorMessage;
        document.getElementById('errorCode').innerHTML = plugin().errorCode;
        }
    catch (e) {
      document.getElementById('errorMessage').innerHTML = plugin().errorMessage;
      document.getElementById('errorCode').innerHTML = plugin().errorCode;
    }
  }
 
  function prepareDataHtml() {	
	document.getElementById('id').innerHTML = plugin().id;
	document.getElementById('CN').innerHTML = plugin().CN;
	document.getElementById('issuerCN').innerHTML = plugin().issuerCN;
	document.getElementById('validFrom').innerHTML = plugin().validFrom;
	document.getElementById('validTo').innerHTML = plugin().validTo;
  }
</script>
 
</head>
<body onload="showVersion(); prepareDataHtml();">
<object id="plugin" type="application/x-digidoc" width="1" height="1"></object>
<!--object id="plugin" classid="clsid:2A4E94A4-B275-491A-9E32-CD7A26FC7C3B" width="1" height="1"></object-->
<br/>
 
Plug-in version: <span id="version"></span>
 
<br/>
<br/>
 
hash: <input type="text" size="80" maxlength="64" id="hash" value="26fd6dbee37c55c14af480f00daff8817b61967e">
<input type="button" onclick="javascript:sign()" value="sign"/>
<br/>
<pre id="signature"></pre>
 
<br/>
<br/>
 
<a href="#" onclick="javascript:showData()">show data</a> <br/>
 
<div>id: <span id="id"></span></div>
<div>CN: <span id="CN"></span></div>
<div>issuerCN: <span id="issuerCN"></span></div>
<div>validFrom: <span id="validFrom"></span></div>
<div>validTo: <span id="validTo"></span></div>
<div>errorMessage: <span id="errorMessage"></span></div>
<div>errorCode: <span id="errorCode"></span></div>
 
<div>PEM:</div>
<pre id="PEM"></pre>
 
<div>DER in hex:</div>
<pre id="hex"></pre>
 
</body>
</html>