var clusterWidth;
var clusterHeight;

var coordYCrop = 0;
var heightCrop = 0;
var currentCrop =  null;

function xmlhttpPost(strURL) {
    var xmlHttpReq = false;
    var self = this;
if( null == readCookie("meta_comp_session_id")){alert("No data available. Please, go to the data input tab"); return;}
if( readCookie("meta_comp_cog_cluster") == null){alert("You must configure clustering parameters"); return;}
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
    self.xmlHttpReq.send(getquerystring());
}

function getquerystring() 
{
    qstr = 'w=' + escape( createClusteringSentence() );  // NOTE: no '?' before querystring
    var fileName = 'file=' + readCookie("meta_comp_session_id");
    return qstr+'&'+fileName;
}

function onEndCrop( coords, dimensions ) 
{
  //coords.x1;
  //coords.y1;
  //coords.x2;
  //coords.y2;
  //dimensions.width;
  //dimensions.height;
  //alert(coords.y1+' '+dimensions.height);
  if( coords.y1 != coordYCrop || dimensions.height != heightCrop )
  {
    coordYCrop = coords.y1;
    heightCrop = dimensions.height;
    zoom(coords.y1,dimensions.height);
  }

}

function zoom( y, height )
{
    var xmlHttpReq = false;
    var self = this;

    document.getElementById("zoom").innerHTML = "<div style=\"position:relative; top:250px;\"><center><img style=\"\" src=\"img/ajax-loader.gif\"/></center></div>";

    // Mozilla/Safari
    if (window.XMLHttpRequest) {
        self.xmlHttpReq = new XMLHttpRequest();
    }
    // IE
    else if (window.ActiveXObject) {
        self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    self.xmlHttpReq.open('POST', "/cgi-bin/commics/CLUSTER/zoom.cgi", true);
    self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    self.xmlHttpReq.onreadystatechange = function() { 
        if (self.xmlHttpReq.readyState == 4) {
                 document.getElementById("zoom").innerHTML = self.xmlHttpReq.responseText;

        }
    }
    var fileName = readCookie("meta_comp_session_id");
    self.xmlHttpReq.send('z='+y+'-'+height+'&'+'file='+fileName);


}

    

function updatepage(str)
{
  
var coordinates = str.substring(0, str.indexOf("|"));

  clusterWidth = parseInt(str.substring(0, str.indexOf("&")));
  clusterHeight = parseInt(str.substring( str.indexOf("&") + 1 ));

  document.getElementById("result").innerHTML = str.substring( str.indexOf("|") +1 );


  if( this.currentCrop == null)
  {
				this.currentCrop = new Cropper.Img( 
					'cluster_image', 
					{ 
						minWidth: clusterWidth, 
                                                maxHeight: 500,
                                                minHeight: 5,
						displayOnInit: true, 
						onEndCrop: onEndCrop 
					} 
				) ;
  }
  else
  {
    this.currentCrop.remove();
    this.currentCrop.initialize(
					'cluster_image', 
					{ 
						minWidth: clusterWidth, 
                                                maxHeight: 500,
                                                minHeight: 5,
						displayOnInit: true, 
						onEndCrop: onEndCrop 
					} 
				);
  }

}

function initWindows(){

    var win = new Window({className:"alphacube", width: 420, height:450});
    win.setContent("config_form_div");  
    win.setZIndex(300);
    loadCookies(document.forms['form_clust_conf']);
  
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
  
function openDialog(html) {
    document.getElementById("config_form_div").style.visibility = "visible";
    var effect = new PopupEffect(html, {className: "popup_effect1"});
    var win = new Window({className:"alphacube", width: 420, height:450, showEffect:effect.show.bind(effect), hideEffect:effect.hide.bind(effect), onClose:closecallback});
    win.setContent("config_form_div");     
    win.setZIndex(300);
    win.setDestroyOnClose(); 
    win.showCenter();


  win.showCenter();
  win.setConstraint(true, {left:0, right:0, top: 30, bottom:10})
  win.toFront();
} 
   
function closecallback(){

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

function eraseCookie(name) 
{
	createCookie(name,"",-1);
}

function IsNumeric(sText)
{
   var ValidChars = "0123456789";
   var IsNumber=true;
   var Char;

   for (i = 0; i < sText.length && IsNumber == true; i++) 
   { 
     Char = sText.charAt(i); 
     if (ValidChars.indexOf(Char) == -1) 
     {
    	 IsNumber = false;
     }
    }
   return IsNumber;
   
}

function IsEmpty(aTextField) 
{
   if ((aTextField.value.length==0) ||
   (aTextField.value==null)) 
	 {
      return true;
   }
   else { return false; }
}


function ValidateForm(form)
{
	if(form.subject4.checked)
	{
		if(IsEmpty(form.subject5)) 
   	{ 
      alert('When running k-means, you have to specify the number of clusters k to use') 
      form.subject5.focus(); 
      return false; 
   	} 
   	if(IsEmpty(form.subject6)) 
   	{ 
      alert('The number of times the k-means clustering algorithm is run must be specified using decimal digits') 
      form.subject6.focus(); 
      return false; 
   	}
 		
		if (!IsNumeric(form.subject5.value)) 
    { 
      alert('The number of clusters k must be specified using decimal digits') 
      form.subject5.focus(); 
      return false; 
    } 

		if (!IsNumeric(form.subject6.value)) 
    { 
      alert('The number of times the k-means clustering algorithm is run must be specified using decimal digits') 
      form.subject6.focus(); 
      return false; 
    }
	}

var i = 0;
while( i < form.subject.length && !form.subject[i].checked ){ i++; }
createCookie("meta_comp_cog_cluster",i+'',365);

i = 0;
while( i < form.subject.length && !form.subject2[i].checked ){ i++; }
createCookie("meta_comp_metagenome_cluster",i+'',365);

i = 0;
while( i < form.subject.length && !form.subject3[i].checked ){ i++; }
createCookie("meta_comp_cluster_method",i+'',365);

if( form.subject4.checked )
{
	createCookie("meta_comp_k_means","true",365);
}
else
{
	createCookie("meta_comp_k_means","false",365);
}

createCookie("meta_comp_num_clusters_k",form.subject5.value,365);
createCookie("meta_comp_times_k_means_run",form.subject6.value,365);

return true;
 
} 

function loadCookies( form )
{
	var index;
  if( readCookie("meta_comp_cog_cluster") != null )
	{
		index = parseInt( readCookie("meta_comp_cog_cluster") );
  	form.subject[index].checked = true;
	}
	
	if( readCookie("meta_comp_metagenome_cluster") != null)
	{
		index = parseInt( readCookie("meta_comp_metagenome_cluster") );
  	form.subject2[index].checked = true;
	}

	if( readCookie("meta_comp_cluster_method") != null )
	{
		index = parseInt( readCookie("meta_comp_cluster_method") );
  	form.subject3[index].checked = true;
	}

	if( readCookie("meta_comp_k_means") != null)
	{
		if( "true" == readCookie("meta_comp_k_means") )
		{
			form.subject4.checked = true;
		}
		else
		{
			form.subject4.checked = false;
		}
	}

	if( readCookie("meta_comp_num_clusters_k") != null )
	{
		form.subject5.value = readCookie("meta_comp_num_clusters_k");
	}

	if( readCookie("meta_comp_times_k_means_run") != null)
	{
  	form.subject6.value = readCookie("meta_comp_times_k_means_run");
	}

}

function createClusteringSentence()
{
  var query = "";
  var index;

  if( readCookie("meta_comp_cog_cluster") != null )
  {
    index = readCookie("meta_comp_cog_cluster");
    query += " -g "+index;
  }
  else
  {
    alert("You must configure the clustering parameters first");
    return null;
  }

  index = readCookie("meta_comp_metagenome_cluster");
  query += " -e "+index;
  
  index = readCookie("meta_comp_cluster_method");
  if( '0' == index )
  {
    query += " -m "+'m';
  }
  else if( '1' == index )
  {
    query += " -m "+'s';
  }
  else if( '2' == index )
  {
    query += " -m "+'c';
  }
  else if( '3' == index )
  {
    query += " -m "+'a';
  }

  if( "true" == readCookie("meta_comp_k_means") )
  {
    query += " -k "+readCookie("meta_comp_num_clusters_k");
    query += " -r "+readCookie("meta_comp_times_k_means_run");
  }
  return query;
}


function divScroll()
{
var Meta = document.getElementById('divMeta');
var Zoom = document.getElementById('divZoom');
var Cog = document.getElementById('divCog');
Meta.scrollLeft = Zoom.scrollLeft;
Cog.scrollTop = Zoom.scrollTop;
}

function clusterScroll()
{
var HTree = document.getElementById('divHTree');
var Cluster = document.getElementById('divCluster');
var VTree = document.getElementById('divVTree');
HTree.scrollLeft = Cluster.scrollLeft;
VTree.scrollTop = Cluster.scrollTop;
}
