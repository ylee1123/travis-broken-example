var express = require('express');
var app = express();

bodyParser = require("body-parser");
app.use(bodyParser.urlencoded({extended: false}));

var fs = require('fs');
var df = require('dataformat');

mysql =require('mysql');

var connection = mysql.createConnection({
	host: 'localhost',
	user: 'sensor2',
	password: '1111',
	database: 'data'
})
connection.connect()

function insert_sensor(user, location, value, user2, serial, ip)
{
	obj = {};
	obj.user = user;
	obj.location = location;
	obj.value = value; // not getting any value
	obj.user2 = user2;
	obj.serial = serial;
	obj.ip = ip;
	obj.date = df(new Date(), "yyyy-mm-dd HH:MM:ss");
	
	var d = JSON.stringify(obj);
	ret = ""+location + user2 + "=" + value;
	
	fs.appendFile("Data.txt", d+'\n', function(err) {
		if(err) console.log("File Write Err: %j",r);
	});
	return(ret);
}

function get_post(cmd, r, req,res)
{
	console.log(cmd+"%j",r);
	ret_msg = "{serial:"+r.serial+",user:"+r.user;
	
	if (r.format == '2') {
		var items = r.items.split(',');
		
		for (var i = 0; i<items.length; i++) {
			if(items[i].length < 3) continue;
			var v = items[i].split('-');
			ret_msg = insert_sensor(r.user, v[3], v[2], v[0], r.serial, req.connection.remoteAddress);
		}
	}
	
	ret_msg += "}";
	res.writeHead(200, {'Content-Type': 'text/plain'});
	res.end("X-ACK" + ret_msg);
}

app.get("/data", function(req, res) {
	console.log("params="+req.query);
	
	var qstr = 'SELECT * FROM sensor where tme > date_sub(now(),INTERVAL 1 DAY)';
	
	connection.query(qstr, function(err, rows, cols) {
		if(err) {
			throw err;
			res.send('query error: '+qstr);
			return;
			}
			
		console.log("Got " + rows.length+ " records");
		var html = "<!doctype html><html><body>";
		html += "<H1> Sensor Data for Last 24 Hours</H1>";
		html += "<table border=1 cellpadding=3 cellspacing=0>";
		html += "<tr><td>Seq#<td>Time Stamp<td>Vib stat<td>Location";
		
		for(var i=0; i<rows.length; i++) {
			html += "<tr><td>"+JSON.stringify(rows[i]['seq']) + 
			"<td>"+JSON.stringify(rows[i]['tme'])+"<td>"+JSON.stringify(rows[i]['value'])+"<td>"+JSON.stringify(rows[i]['location']);
						}
			html += "</table>";
			html+="</body></html>";
			res.send(html);
		});
	});
	
app.get('/logone', function(req, res){
	var i = 1;
	i++;
	
	r = {};
	r.seq = i;
	r.device='101';
	r.unit='0';
	r.location=req.query.gps;
	r.ip = req.ip;
	r.value = req.query.data;
	
	var query = connection.query("insert into sensor set ?", r, function(err, rows, cols) {
		//'INSERT INTO sensorLog(sensorId, logTime, sensorValue) VALUES(4, current_timestamp(), ' + temp + ')'
		if(err) {
			throw err;
		}
		console.log("[+]SQL injection is done!");
	});
	
	var date = new Date();
	fs.appendFile("log.txt", JSON.stringify(req.query)+", "+req.ip+", "+date+"\n", function(err){
		
		if(err) {
			return console.log(err);
		}
	})
	r = req.query;
	get_post("GET", r, req, res);
});

app.post('/logone', function(req, res){
	r= {};
	r.seq=1;
	r.location=req.query.gps;
	r.device='101';
	r.unit='0';
	r.ip = req.ip;
	r.value = req.query.data;
	
	var query = connection.query("insert into sensor set ?",r,function(err,rows,cols) {
		if(err) {
			throw err;
			}
		console.log("done");
		});
		
	var date = new Date();
	
	fs.appendFile("log.txt",JSON.stringify(req.query)+","+req.ip+","+date+"\n",function(err){
		if(err){
			return console.log(err);
		}
	})
	
	r = req.body;
	get_post("POST",r,req,res);
	
});


var server = app.listen(3000, function() {
	var host = server.address().address
	var port = server.address().port
	console.log('listening at http://%s:%s',host,port)
});

	
			
