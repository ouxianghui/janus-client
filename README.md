## janus-client

'janus-client' is an open source Janus Gateway client developed with webrtc native C++ for learning and research purposes. Most components are based on 'rtc_base' of WebRTC. MacOS, IOS and Linux platforms will be supported after the main features are completed.

## Dependencies

* [Qt5](http://download.qt.io/archive/qt/) Used Qt for UI
* [asio](https://github.com/chriskohlhoff/asio) Referenced by websocketpp
* [websocketpp](https://github.com/zaphoyd/websocketpp) WebSockets support for the Janus API
* [rapidjson](https://github.com/Tencent/rapidjson.git) Used for conversion between C++ objects and json
* [glew](http://glew.sourceforge.net/) Used to render video frames
* [spdlog](https://github.com/gabime/spdlog) Used to build a log system
* WebRTC

* *Note:* 
* asio/websocketpp/rapidjson/spdlog have joined this repository as submodules. 
* glew and WebRTC library(M96, debug/release, include/lib/bin) have been included directly in this repository. 

## Features
* video room (available)
* text room
* video call
* voice mail

## Arch
<img src="https://github.com/ouxianghui/janus_client/blob/main/arch.svg" height="500" /><br>

## Compile
Get the code:

	git clone --recursive https://github.com/ouxianghui/janus-client.git
  
  Open RTCSln.sln with Visual Studio(2019)
  
## Server

* [janus-gateway](https://github.com/meetecho/janus-gateway.git) Checkout the 'multistream' branch


## Author
Jackie Ou 750265900@qq.com

Any thought, suggestion, feedback is welcome!
