/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#ifndef GALLERY_VIEW_H
#define GALLERY_VIEW_H

#include <QFrame>
#include <vector>
#include <algorithm>
#include "api/media_stream_interface.h"
#include "gl_video_renderer.h"

namespace Ui {
class GalleryView;
}

class QGridLayout;

namespace webrtc {
	class VideoTrackInterface;
}

class IContentView {
public:
	virtual ~IContentView() {}
	virtual bool init() = 0;
	virtual void cleanup() = 0;
	virtual int64_t id() = 0;
	virtual QWidget* view() = 0;
};

class ContentView : public IContentView {

public:
	ContentView(int64_t id, rtc::scoped_refptr<webrtc::VideoTrackInterface> track, GLVideoRenderer* renderer)
	: _id(id)
	, _track(track)
	, _renderer(renderer) {

	}
	bool init() override {
		if (_renderer && _track) {
			rtc::VideoSinkWants wants;
			_track->AddOrUpdateSink(_renderer, wants);
			return true;
		}
		return false;
	}
	void cleanup() override {
		if (_renderer && _track) {
			_track->RemoveSink(_renderer);
		}
	}
	int64_t id() override {
		return _id;
	}
	QWidget* view() override {
		return static_cast<QWidget*>(_renderer);
	}
private:
    int64_t _id = -1;
	rtc::scoped_refptr<webrtc::VideoTrackInterface> _track;
	GLVideoRenderer* _renderer = nullptr;
};

class PermuteStrategy {
public:
    virtual ~PermuteStrategy() {}

    virtual void permute(std::vector<std::shared_ptr<IContentView>>& views) = 0;
};

class DefaultStrategy : public PermuteStrategy {
public:
    DefaultStrategy() {}

    void permute(std::vector<std::shared_ptr<IContentView>>& views) override
    {
        std::sort(views.begin(), views.end(), [](const auto& e1, const auto& e2){
            return e1->id() < e2->id();
        });
    }
};

class GalleryView : public QFrame
{
    Q_OBJECT

    enum class Strategy : int {
      DEFAULT = 0
    };

public:
    explicit GalleryView(QWidget *parent = nullptr);

    ~GalleryView();

    void insertView(std::shared_ptr<ContentView> view);

    void removeView(int64_t id);

    QWidget* getView(int64_t id);

    void removeAll();

protected:
    void init();

    std::shared_ptr<PermuteStrategy> getPermuteStrategy(Strategy strategys);

    void permute(Strategy strategy = Strategy::DEFAULT);

    void permuteViews();

private:
    Ui::GalleryView *ui;

    QGridLayout* _gridLayout;

    std::vector<std::shared_ptr<IContentView>> _views;
};

#endif // GALLERY_VIEW_H
