/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "gallery_view.h"
#include "ui_gallery_view.h"
#include <QGridLayout>
#include "gl_video_renderer.h"

GalleryView::GalleryView(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GalleryView)
{
    ui->setupUi(this);
    init();
}

GalleryView::~GalleryView()
{
	removeAll();
    delete ui;
}

void GalleryView::init()
{
    setFrameShape(QFrame::Shape::WinPanel);

    _gridLayout = new QGridLayout(this);

    this->setLayout(_gridLayout);

    _gridLayout->setAlignment(Qt::AlignCenter);
}

void GalleryView::insertView(std::shared_ptr<ContentView> view)
{
    bool found = std::any_of(_views.begin(), _views.end(), [view](const auto& e) {
        return e->id() == view->id();
    });
    if(!found) {
        _views.emplace_back(view);
        permuteViews();
    }
}

void GalleryView::removeView(int64_t id)
{
    auto it = std::remove_if(_views.begin(), _views.end(), [id](const auto& e) {
        return e->id() == id;
    });
    if (it != _views.end()) {
        _views.erase(it);
        permuteViews();
    }
}

QWidget* GalleryView::getView(int64_t id)
{
    auto it = std::find_if(_views.begin(), _views.end(), [id](const auto& e) {
       return id == e->id();
    });
    if (it != _views.end()) {
        return (*it)->view();
    }
    return nullptr;
}

std::shared_ptr<PermuteStrategy> GalleryView::getPermuteStrategy(Strategy strategy)
{
    if (strategy == Strategy::DEFAULT) {
        return std::make_shared<DefaultStrategy>();
    }
    return nullptr;
}

void GalleryView::removeAll()
{
	for (auto v : _views) {
		v->cleanup();
	}
    _views.clear();
    permuteViews();
}

void GalleryView::permute(Strategy strategy)
{
    if (auto ds = getPermuteStrategy(strategy)) {
        ds->permute(_views);
    }
    update();
}

void GalleryView::permuteViews()
{
    QLayoutItem* child = nullptr;
    while ((child = _gridLayout->takeAt(0)) != nullptr) {
        _gridLayout->removeWidget(child->widget());
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }

    if (_views.empty()) {
        return;
    }

    permute();

    uint32_t size = static_cast<uint32_t>(_views.size());
    int column = static_cast<int>(std::ceil(std::sqrtf(size)));
    int row =  static_cast<int>(std::ceil(double(size)/double(column)));

    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < column; ++c) {
            uint32_t index = static_cast<uint32_t>(r * column + c);
            if (index < size) {
                _views[index]->view()->setParent(this);
                _gridLayout->addWidget(_views[index]->view(), r, c);
            }
        }
    }

}
