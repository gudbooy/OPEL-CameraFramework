{
  "targets": [
    {
      "target_name": "OPELRecording",
      "sources": ["src/OPELRecording.cc", "src/OPELRecordingAPI.cc"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
			"cflags" : ["-Wno-unused-variable"],
			"cflags!" : ["-fno-exceptions"],
			"cflags_cc!" : ["-fno-exceptions"],
			"libraries" : ["<!@(pkg-config --cflags dbus-1)"],
			"cflags" : ["<!@(pkg-config --cflags dbus-1)"],
			"cflags_cc" : ["<!@(pkg-config --cflags dbus-1)"], 
			"cflags_cc!" : ["-fno-rtti"],
			"cflags_cc+" : ["-frtti"]
    }
  ]
}
