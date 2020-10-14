/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "gl_defines.h"
#include <memory>
#include <vector>
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include <stdint.h>

// Two sets of 3 textures are used here, one for each of the Y, U and V planes. Having two sets
// alleviates CPU blockage in the event that the GPU is asked to render to a texture that is already
// in use.
static const GLsizei kNumTextureSets = 2;
static const GLsizei kNumTexturesPerSet = 3;
static const GLsizei kNumTextures = kNumTexturesPerSet * kNumTextureSets;

class I420TextureCache 
	: public std::enable_shared_from_this<I420TextureCache>
{
public:
	I420TextureCache();
	~I420TextureCache();

public:
	void init(); 

	void uploadFrameToTextures(const webrtc::VideoFrame& frame);

	GLuint yTexture();

	GLuint uTexture();

	GLuint vTexture();

protected:
	void setupTextures();

	void uploadPlane(const uint8_t* plane, GLuint texture, size_t width, size_t height, int32_t stride);

private:
	bool _hasUnpackRowLength;
	GLint _currentTextureSet = 0;
	// Handles for OpenGL constructs.
	GLuint _textures[kNumTextures];
	// Used to create a non-padded plane for GPU upload when we receive padded frames.
	std::vector<uint8_t> _planeBuffer;
};

