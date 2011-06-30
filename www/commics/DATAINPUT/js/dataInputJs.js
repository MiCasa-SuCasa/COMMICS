function xmlhttpPost(strURL) {
    var xmlHttpReq = false;
    var self = this;

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

        }
    }
    self.xmlHttpReq.send();
}

function updatepage(str)
{
  

  document.getElementById("divMetaTable").innerHTML = str;


				mySorted = new SortedTable();
				mySorted.colorize = function() {
					for (var i=0;i<this.elements.length;i++) {
						if (i%2){
							this.changeClass(this.elements[i],'even','odd');
						} else {
							this.changeClass(this.elements[i],'odd','even');
						}
					}
				}
				mySorted.onsort = mySorted.colorize;
				mySorted.onmove = mySorted.colorize;
				mySorted.colorize();

  var cookieVal = readCookie("meta_comp_session_id");
  if( null == cookieVal )
  {
    document.getElementById("dataStatus").innerHTML = "No data set..";
    document.getElementById("dataStatus").style.backgroundColor="red";
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

function createCookie(name,value,days) 
{
	if (days) 
  {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function submitInputdata()
{
    var sessionId = generateSessionId();
    createCookie("meta_comp_session_id", sessionId,null); //we create a session cookie

    var xmlHttpReq = false;
    var self = this;
    var strURL = '/cgi-bin/commics/DATAINPUT/generateData.cgi';
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
            updateDataStatus(self.xmlHttpReq.responseText);
Dialog.closeInfo();

        }
    }
    var customData = document.getElementById('custom_data').value;
    var customCogsData = document.getElementById('custom_cog_data').value;
    self.xmlHttpReq.send( 'data='+SortedTable.SelectedItems (document.getElementById('row1'))+'&categories='+SortedTable.SelectedItems (document.getElementById('row1Categories'))+'&sessionId='+sessionId+'&customData='+customData+'&customCogs='+customCogsData );

}

function generateSessionId()
{
  var randomNumber = Math.floor(Math.random()*101);
  var currentTime = new Date()
  var time = currentTime.getTime();
  return randomNumber+'_'+time;
}

function updateDataStatus( responseText)
{
document.getElementById("dataStatus").innerHTML = "Data ready!";
document.getElementById("dataStatus").style.backgroundColor="green"
}

function openHelpCustomDialog(html) {
var containerOffset = document.getElementById('metagenomes').offsetTop;
    var effect = new PopupEffect(html, {className: "popup_effect1"});
    var win = new Window({className:"alphacube", width: 350, height:200, top:html.offsetTop+containerOffset, left:html.offsetLeft+200, showEffect:effect.show.bind(effect), hideEffect:effect.hide.bind(effect), onClose:closecallback});

    win.getContent().innerHTML= "<div style='padding:10px'>You can enter your own data using the structure of the following example:<br/> <pre style='font-weight:bold; border:1px inset'>>ANAS dechlorinating bioreactor (Sample 196)<br/>TOTAL_GENES 61557<br/>COG0001 21<br/>COG0002 24<br/>COG0003 8<br/>COG0004 22<br/>...</pre>\
</div>"

    //win.setContent("config_form_div");     
    win.setZIndex(300);
    win.setDestroyOnClose(); 
    //win.showCenter();
win.show();
// html.offsetTop   html.offsetLeft
  //win.showCenter();
  //win.setConstraint(true, {left:0, right:0, top: 30, bottom:10})
  win.toFront();
} 

function openHelpCogsDialog(mhtml) {

var containerOffset = document.getElementById('categories').offsetTop;

    var effect = new PopupEffect(mhtml, {className: "popup_effect1"});
    var win = new Window({className:"alphacube", width: 250, height:120, top:mhtml.offsetTop+containerOffset, left:mhtml.offsetLeft+200, showEffect:effect.show.bind(effect), hideEffect:effect.hide.bind(effect), onClose:closecallback});

    win.getContent().innerHTML= "<div style='padding:10px'> You can additionally (or uniquely) specify a list of COGS, one per row and without blank spaces.</div>"
  
    win.setZIndex(300);
    win.setDestroyOnClose(); 
win.show();
  win.toFront();
} 
function closecallback(){

}
