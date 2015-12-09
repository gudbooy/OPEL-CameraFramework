console.log("Start Code");
var addon = require('./build/Release/OPELRecording');
var recObj = new addon.OPELRecording();

recObj.init(function(width, height, buffer_size){ 
		console.log('width : ', width);
		console.log('height : ', height);
		console.log('buffer_size : ', buffer_size);});
 
recObj.start("/home/pi/camData/some.264", 20, function(error, data){
		console.log('data : ', data);			
		console.log('done!!!!!\n');
		});

for(;;)
{//	console.log("hihi");
}
