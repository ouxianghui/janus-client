/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "gl_video_renderer.h"
#include "gl_video_shader.h"
#include "i420_texture_cache.h"
#include "logger/logger.h"
#include <thread>
#include "absl/types/optional.h"
#include "api/video/video_rotation.h"
#include <array>
#include "common_video/libyuv/include/webrtc_libyuv.h"

GLVideoRenderer::GLVideoRenderer(QWidget *parent)
	: QOpenGLWidget(parent)
{

}

GLVideoRenderer::~GLVideoRenderer()
{

}

void GLVideoRenderer::init()
{
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet("background-color:rgb(255, 0, 255)");
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_videoShader = std::make_shared<GLVideoShader>();
}

void GLVideoRenderer::initializeGL() 
{
	initializeOpenGLFunctions();
	glewInit();
	glEnable(GL_DEPTH_TEST);
	// Set up the rendering context, load shaders and other resources, etc.:
	//QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
}

void GLVideoRenderer::resizeGL(int w, int h)
{
	std::lock_guard<std::mutex> lock(_mutex);
	// Update projection matrix and other size related settings:
	if (_frame) {
		glViewport(0, 0, _frame->width(), _frame->height());
	}
	else {
		glViewport(0, 0, 640, 480);
	}
}

void GLVideoRenderer::paintGL()
{
	if (!_i420TextureCache) {
		_i420TextureCache = std::make_shared<I420TextureCache>();
		_i420TextureCache->init();
	}
	std::lock_guard<std::mutex> lock(_mutex);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	if (_frame) {
		_i420TextureCache->uploadFrameToTextures(*_frame);
		_videoShader->applyShadingForFrame(_frame->width(),
			_frame->height(),
			_frame->rotation(),
			_i420TextureCache->yTexture(),
			_i420TextureCache->uTexture(),
			_i420TextureCache->vTexture());
	}
}

void GLVideoRenderer::OnFrame(const webrtc::VideoFrame& frame)
{
	//std::lock_guard<std::mutex> lock(_mutex);
	_frame = std::make_shared<webrtc::VideoFrame>(frame);
	static int counter = 0;
	//DLOG("--> frame: {}, ts: {}", ++counter, _frame->timestamp_us());

	update();
}

void GLVideoRenderer::resizeEvent(QResizeEvent *event)
{
	QOpenGLWidget::resizeEvent(event);
}
