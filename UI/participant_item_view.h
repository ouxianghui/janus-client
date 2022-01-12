#pragma once

#include <QWidget>
#include "ui_participant_item_view.h"
#include <memory>


namespace vi {
    class Participant;
    class VideoRoomClientInterface;
}

class ParticipantItemView : public QWidget
{
	Q_OBJECT

public:
    ParticipantItemView(std::shared_ptr<vi::Participant> participant, std::shared_ptr<vi::VideoRoomClientInterface> vrc, QWidget *parent = Q_NULLPTR);

	~ParticipantItemView();

    void setDisplayName(const std::string& name);

    std::string getDisplayName();

    void setAudioStatus(bool mute);

    bool getAudioStatus() const;

    void setVideoStatus(bool mute);

    bool getVideoStatus() const;

private slots:
    void on_toolButtonAudio_clicked(bool checked);

    void on_toolButtonVideo_clicked(bool checked);

private:
	Ui::ParticipantItemView ui;

    std::weak_ptr<vi::Participant> _participant;

    std::weak_ptr<vi::VideoRoomClientInterface> _vrc;
};
