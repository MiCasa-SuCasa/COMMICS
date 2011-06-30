function xmlhttpPost(strURL) {
    var xmlHttpReq = false;
    var self = this;
if( null == readCookie("meta_comp_session_id")){alert("No data available. Please, go to the data input tab"); return;}
Dialog.info("Working...",{width:250, height:100, showProgress: true, zindex: 300});

    // Mozilla/Safari
    if (window.XMLHttpRequest) {
        self.xmlHttpReq = new XMLHttpRequest();
    }
    // IE
    else if (window.ActiveXObject) {
        self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    self.xmlHttpReq.open('POST', strURL, true);
    self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    self.xmlHttpReq.onreadystatechange = function() {
        if (self.xmlHttpReq.readyState == 4) {
            updatepage(self.xmlHttpReq.responseText);
Dialog.closeInfo();
        }
    }
    self.xmlHttpReq.send('file='+"cda_"+readCookie("meta_comp_session_id")+".txt"+"&type="+document.getElementById("envType").value);

}


function updatepage(str)
{
  

  document.getElementById("result").innerHTML = str;




}

function initWindows(){

    var dataName = readCookie("meta_comp_session_id");
    if( null == dataName)
    {
    }
    else
    {
      document.getElementById("dataStatus").innerHTML = "Data ready!";
      document.getElementById("dataStatus").style.backgroundColor="green";
    }

}

function readCookie(name) 
{
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++)
        {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}
