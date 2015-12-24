require('date-utils');
var OPELCameraAddon = require('./build/Release/OPELRecording');
var BlackBox = new OPELCameraAddon.OPELRecording();

var dt;
var d;

dt = new Date();
d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');

var path = '/home/pi/BlackBox/';

BlackBox.init(function(width, height, buffer_size){
		console.log("BlackBox Setting");
		console.log('width : ', width);
		console.log('height : ', height);
		console.log('FPS : 30');
		});
	
//synchronous Motion Detection Code
//if motion detected
BlackBox.start(path+'['+d+'].mjpg', 300/*300frame: 10sec*/, function(err, data){
			if(data)
					console.log(path+'['+d+'].mjpeg Video Saved');
		});
