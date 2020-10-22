## janus_client

Janus_client is an open source, learning and research purpose, C/C++ native designed and developed by Jackie Ou. Most components of janus_client are based on rtc_base of WebRTC. In order to debug webrtc code directly, I choose to use Visual Studio to develop on Windows platform. MacOS, IOS and Linux platforms will be supported after the main feautres are completed.

## Dependencies

* [Qt5](http://download.qt.io/archive/qt/) Used Qt for UI
* [asio](https://github.com/chriskohlhoff/asio) Referenced by websocketpp
* [websocketpp](https://github.com/zaphoyd/websocketpp) WebSockets support for the Janus API
* [x2struct](https://github.com/xyz347/x2struct/) Used to convert between C++ objects and json
* [glew](http://glew.sourceforge.net/) Used to render video frames
* [spdlog](https://github.com/gabime/spdlog) Used to build a log system
* WebRTC

* *Note:* 
* asio/websocketpp/x2struct/spdlog have joined this repository as submodules. 
* glew and WebRTC library(M85, release, webrtc.7z) have been included directly in this repository. 

## Features
* video room (in progress)
* text room
* video call
* voice mail

## Arch
<img src="https://github.com/ouxianghui/janus_client/blob/main/janus-client-arch.svg" height="500" /><br>

## Compile
Get the code:

	git clone --recursive https://github.com/ouxianghui/janus_client.git
  
  Open RTCSln.sln with Visual Studio(2017)
  
## Author
Jackie Ou 750265900@qq.com

Any thought, suggestion, feedback is welcome!
