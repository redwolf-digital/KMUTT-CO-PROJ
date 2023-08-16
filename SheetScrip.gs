var sheet_id = "SHEET ID";
var sheet_name = "SHEET NANE";

function doGet(e){
var ss = SpreadsheetApp.openById(sheet_id);
var sheet = ss.getSheetByName(sheet_name);

var date = String(e.parameter.date);
var time = String(e.parameter.time);

var lat = Number(e.parameter.lat);
var lon = Number(e.parameter.lon);

var n1x = Number(e.parameter.n1x);
var n1y = Number(e.parameter.n1y);
var n1z = Number(e.parameter.n1z);
var temp1 = Number(e.parameter.temp1);
var humi1 = Number(e.parameter.humi1);

var n2x = Number(e.parameter.n2x);
var n2y = Number(e.parameter.n2y);
var n2z = Number(e.parameter.n2z);
var temp2 = Number(e.parameter.temp2);
var humi2 = Number(e.parameter.humi2);

var n3x = Number(e.parameter.n3x);
var n3y = Number(e.parameter.n3y);
var n3z = Number(e.parameter.n3z);
var temp3 = Number(e.parameter.temp3);
var humi3 = Number(e.parameter.humi3);

ver gpsstatus = Number(e.page.gpsstatus);
var n1status = Number(e.page.n1status);
var n2status = Number(e.page.n2status);
var n3status = Number(e.page.n3status);
var n4status = Number(e.page.n4status);
var n5status = Number(e.page.n5status);


sheet.appendRow([date,time,lat,lon,n1x,n1y,n1z,temp1,humi1,n2x,n2y,n2z,temp2,humi2,n3x,n3y,n3z,temp3,humi3,gpsstatus,n1status,n2status,n3status,n4status,n5status]);
}
