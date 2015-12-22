var addon = require('./build/Release/OPELFaceDetection');

var count = 10;
addon.faceDetection(count, function(err, numberOfFace){
		if(numberOfFace > 0)
				console.log('Face Detection !!! \n');
				console.log('Number of Face : ' + numberOfFace +'\n');
		});

