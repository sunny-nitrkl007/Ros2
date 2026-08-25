

var tabLinks = new Array();
var contentDivs = new Array();

//Create a boolean variable that is use to make the XmlHttp requests
var xmlhttp = false;

// This is the 
/*
 * Constant string that represents parameters to be requested.
 * @const
 * @type {string}
 */

var requestString = 'cgi-bin/jhm2CgiApp.cgi?requestType=ParameterRequest&paramsRequested=EastingTop,EastingBottom,NorthingTop,NorthingBottom,Elevation';


var cmdString = 'cgi-bin/jhm2CgiApp.cgi?requestType=CommandRequest&commandRequested=ExecuteButtonPress';



  //===============================================================================
//S e t u p   P a g e 
//===============================================================================
/*
* First function that gets called when the page loads.  This function
* sets up the xmlhttp object, periodic refresh of the parameter data
* and kicks off the first parameter data update.
*/
function SetupPage()
{

   // 
   // Check if we are using IE.
   //
   try
   {
      // If the Javascript version is greater than 5.
      xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
   }
   catch (e)
   {
      // If not, then use the older active x object.
      try
      {
         // If we are using Internet Explorer.
         xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
      }
      catch (E)
      {
         // Else we must be using a non-IE browser.
         xmlhttp = false;
      }
   }


   // If we are using a non-IE browser, create an instance
   // of the HMLHttpRequest object.
   if (!xmlhttp && typeof XMLHttpRequest != 'undefined')
   {
      xmlhttp = new XMLHttpRequest();
   }

   // Set up a function that gets called when
   // the state of the xmlhttp object changes.
   xmlhttp.onreadystatechange = function()
   {
      HttpStateChange();
   };

   // Set up a periodic function that refreshes the parameters every second.
   setInterval(function()
   {
      makerequest(requestString);
   }, 1000);

   // Go ahead and make the first request now
   makerequest(requestString);

   return;
}



//====================================================================
//M a k e   R e q u e s t
//====================================================================
/*
* Function that uses the xmlhttp object to request parameter data
*/
function makerequest(serverPage)
{
   xmlhttp.open("GET", serverPage);
   xmlhttp.send(null);
   return;
}



//====================================================================
//  H t t p   S t a t e   C h a n g e 
//====================================================================
/*
* Function that gets called when the state of the xmlhttp object
* changes. 
*/
function HttpStateChange()
{
   // We only do something if the xmlhttp object is ready for us to read from it.
   if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
   {
      // Get the XML that was returned as an Xml Document
      var xmlDoc = xmlhttp.responseXML;
     
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // WE MAY NEED THE NEXT LINE FOR INTERNET EXPLORER????
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!      
      // Configure the document to use XPath (needed for Internet Explorer)
      //xmlDoc.setProperty("SelectionLanguage","XPath");

      // If we received any XML data then update the page with the data
      // we received
      if( xmlDoc != null)
      {
         UpdatePageWithXmlData(xmlDoc);   
      }     
   }

   return;
}

/*
 * Function to get the value of one element out of the passed XML document.
 * 
 * @param xmlDoc {} XML that is to be parsed
 * @param valuePath {string} Path that points to the part of the document we want to parse
 * @param scaling {number} Optional parameter that indicates how the value parsed should be scaled
 * @param decimalPoints {number} Optional parameter that indicates the maximum num of digits after the decimal point
 * @param maxValue {number} Optional parameter that indicates the highest possible value that we want the return value to have
 * @param maxValueText {string} Optional parameter that indicates what value to return if the parsed value is greater than the max value.
 * @return {string|number}
 *   
 */
function GetElement(xmlDoc, valuePath, scaling, decimalPoints, maxValue, maxValueText)
{
	// Get the XPathResult from the XML document
	var xmlResult = xmlDoc.evaluate(
			valuePath,
			xmlDoc.documentElement,
			null,
			XPathResult.STRING_TYPE,
			null);
	
	// Get the actual string value that was found...
	var returnValue = xmlResult.stringValue;

	// See if we are going to scale or round the value, if so we
	// need to convert it to a number before we can work with it.
	var scaled = false;
	var round = false;
	
	// If the scaling parameter is defined then we are scaling
	if( typeof scaling != 'undefined' )
	{
		scaled = true;
	}
	
	// If the decimal points parameter is defined then we are rounding...
	if( typeof decimalPoints != 'undefined' )
	{
		round = true;
	}
	if( (scaled === true) || (round === true) )
	{
		//Convert the string to a float
		returnValue = +returnValue;
	}
	
	// Scale the value, if we are asked to...
	if( scaled === true)
    {
		// This works fine if for some reason return value is NAN
		returnValue = returnValue * scaling;
    }
	
	// Round the value to the appropriate decimal place if we were asked to
	if( round === true)
    {
		// This will throw an exception of returnValue is NaN
		if(!isNaN(returnValue))
        {
		   returnValue = returnValue.toFixed(decimalPoints);
        }
    }

	// See if a max value was specified, if
	// so the return value if it is too large.
	if( typeof maxValue != 'undefined')
    {
		// We can assume that returnValue is a number here.
        if( returnValue > maxValue)
        {
        	returnValue = maxValueText;
        }
    }
	
	/*
	// The toString method exists even if returnValue is still a string...
	return(returnValue.toString());
	*/
   return(returnValue);
}

/*
 * Function that updates an element in the HTML document (div or span)
 * 
 * @param docElement {string} Name of the HTML document element that is to be updated.
 * @param valueString {string} Value that the HTML document element is to be updated with.
 *  
 */
function UpdateDocElement(docElement, valueString)
{
	document.getElementById(docElement).innerHTML = valueString;
	return;
}

/*
 * Function that updates the HTML document using a value that was parsed out of the XML
 * 
 * @param xmlDoc {} XML that is to be parsed
 * @param valuePath {string} Path that points to the part of the document we want to parse
 * @param docElement {string} Name of the HTML document element that is to be updated.
 * @param scaling {number} Optional parameter that indicates how the value parsed should be scaled
 * @param decimalPoints {number} Optional parameter that indicates the maximum num of digits after the decimal point
 * @param maxValue {number} Optional parameter that indicates the highest possible value that we want the return value to have
 * @param maxValueText {string} Optional parameter that indicates what value to return if the parsed value is greater than the max value.
 * @return {string|number}
 */
function UpdateDocElementFromXPath(xmlDoc, valuePath, docElement, scaling, decimalPoints, maxValue, maxValueText)
{
	valueString = GetElement(xmlDoc, valuePath, scaling, decimalPoints, maxValue, maxValueText);
	UpdateDocElement(docElement, valueString);
	return;
}

/*
 * Function that updates the HTML document with the value of a SimpleParamer type XML element.
 * 
 * @param xmlDoc {} XML that is to be parsed
 * @param elementName {string} Name of the SimpleParam element we are going to update the HTML document with
 * @param scaling {number} Optional parameter that indicates how the value parsed should be scaled
 * @param decimalPoints {number} Optional parameter that indicates the maximum num of digits after the decimal point
 * @param maxValue {number} Optional parameter that indicates the highest possible value that we want the return value to have
 * @param maxValueText {string} Optional parameter that indicates what value to return if the parsed value is greater than the max value. 
 * 
 */
function UpdateDocSimpleElement(xmlDoc, elementName, scaling, decimalPoints, maxValue, maxValueText)
{
	xmlPath = "Param[@name='" + elementName + "']/Value";
	UpdateDocElementFromXPath(xmlDoc, xmlPath, elementName, scaling, decimalPoints, maxValue, maxValueText);
	return;
}


function StringToBoolean(stringVal)
{

   switch(stringVal.toLowerCase())
   {
      case "true":
      {
         return(true);
         break;
      }
      case "false":
      {
         return(false);
         break;
      }
      case "":
      {
         return(false);
         break;
      }
      default:
      {
         return(true);
         break;
      }
   }
}

function GetSimpleNumberValue(xmlDoc, elementName)
{
	xmlPath = "Param[@name='" + elementName + "']/Value";
	// Get the XPathResult from the XML document
	var xmlResult = xmlDoc.evaluate(
			xmlPath,
			xmlDoc.documentElement,
			null,
			XPathResult.NUMBER_TYPE,
			null);
	
	return returnValue = xmlResult.numberValue;	
}

function GetSimpleStringValue(xmlDoc, elementName)
{
	xmlPath = "Param[@name='" + elementName + "']/Value";
	// Get the XPathResult from the XML document
	var xmlResult = xmlDoc.evaluate(
			xmlPath,
			xmlDoc.documentElement,
			null,
			XPathResult.STRING_TYPE,
			null);
	
	return returnValue = xmlResult.stringValue;	
}

/*
 * Main function that updates the Web page with XML data.
 * 
 * param xmlDoc {} Xml Data that will be used to parse out data to update the web page with.
 *  
 */
function UpdatePageWithXmlData(xmlDoc)
{

   UpdateDocSimpleElement(xmlDoc, 'EastingTop');
   UpdateDocSimpleElement(xmlDoc, 'EastingBottom');
   UpdateDocSimpleElement(xmlDoc, 'NorthingTop');
   UpdateDocSimpleElement(xmlDoc, 'NorthingBottom');
   UpdateDocSimpleElement(xmlDoc, 'Elevation');
  
}
