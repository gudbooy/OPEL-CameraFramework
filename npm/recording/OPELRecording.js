console.log("Start Code");
var addon = require('./build/Release/OPELRecording');
var recObj = new addon.OPELRecording();

recObj.init(function(width, height, buffer_size){ 
		console.log('width : ', width);
		console.log('height : ', height);
		console.log('buffer_size : ', buffer_size);});
 
recObj.start("/home/mini/camData/some.264", 200, function(error, data){
		
	});
