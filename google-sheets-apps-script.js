//https://script.google.com/macros/s/AKfycbxNyCYvRRHDYDFITyw37Z0JzpnavpdqC0gZuDIihgVCfDMLF0FjpFlTE_bd55bocg_B/exec?temp=25&hum=50&co2=556


/**
 * doGet() function to add data to a spreadsheet.
 *
 * Spreadsheet data is provided as a querystring, e.g. ?col1=1&col2='pizza'
 *
 * From: http://stackoverflow.com/a/18725479/1677912
 *
 * @param {event} e Event passed to doGet, with querystring
 * @returns {String/html} Html to be served
 *
 * Test URLs (adjust ID as needed):
 *   https://script.google.com/macros/s/--DEV-SCRIPT-ID--/dev?col1=1&col2='pizza'
 *   https://script.google.com/macros/s/--PUB-SCRIPT-ID--/exec?col1=1&col2='pizza'
 */

//

function doGet(e) {  
  Logger.log( JSON.stringify(e) );  // view parameters

  var result = 'Ok'; // assume success

  if (e.parameter == undefined) {
    result = 'No Parameters';
  }
  else {
    var id = '1fZptMdE8rzTCnRQb0GRXm-d9UvsZGKWTOV6Pf7JP454'; // Spreadsheet id for responses
    var sheet = SpreadsheetApp.openById(id).getActiveSheet();

    // get Header row range
    var headerRow = sheet.getRange(1, 1, 1, 6);

    // add Header values
    headerRow.setValues([['Date', 'Time', 'PHASE', 'Temperature', 'Humidity', 'CO2']]);

    var newRow = sheet.getLastRow() + 1;
    var rowData = [];
    var d = new Date();
    rowData[0] = d; // Timestamp
    rowData[1] = d.toLocaleTimeString();
    
    var lastPhase = sheet.getRange(sheet.getLastRow(), 3);
    if(lastPhase.getDisplayValue() == "PHASE" || lastPhase.isBlank())
      rowData[2] = "DARK"; // Or 'HARVEST-1', 'HARVEST-2', 'HARVEST-3' 
    else
      rowData[2] = lastPhase.getDisplayValue();  

    for (var param in e.parameter) {
      Logger.log('In for loop, param='+param);
      var value = stripQuotes(e.parameter[param]);
      //Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'temp': 
          rowData[3] = value;
          break;
        case 'hum':
          rowData[4] = value;
          break;
        case 'co2':
          rowData[5] = value;
          break;
        default:
          result = "unsupported parameter";
      }
    }
    Logger.log(JSON.stringify(rowData));

    // Write new row to spreadsheet
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);

    // Return result of operation
    return ContentService.createTextOutput(rowData[2]);
  }
  // Return result of operation
  return ContentService.createTextOutput("No output");
}
/*
function doPost(e) {
  var read = e.parameter.read;
  if (read !== undefined){
    return ContentService.createTextOutput(sheet.getRange(sheet.getLastRow(), 3).getDisplayValue());
  }
  
  return ContentService.createTextOutput("No value passed as argument to script Url.");
}
*/
/**
 * Remove leading and trailing single or double quotes
 */
function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}


