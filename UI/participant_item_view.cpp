#include "participant_item_view.h"
#include "participant.h"
#include "video_room_client_interface.h"
#include "logger/logger.h"
#include "media_controller_interface.h"

ParticipantItemView::ParticipantItemView(std::shared_ptr<vi::Participant> participant, std::shared_ptr<vi::VideoRoomClientInterface> vrc, QWidget* parent)
    : QWidget(parent)
    , _participant(participant)
    , _vrc(vrc)
{
    ui.setupUi(this);
}

ParticipantItemView::~ParticipantItemView()
{
}

void ParticipantItemView::setDisplayName(const std::string& name)
{
    ui.labelDisplayName->setText(name.c_str());
}

std::string ParticipantItemView::getDisplayName()
{
    return ui.labelDisplayName->text().toStdString();
}

void ParticipantItemView::setAudioStatus(bool mute)
{
    //auto participant = _participant.lock();
    //if (!participant) {
    //    return;
    //}

    //if (mute) {
    //    participant->muteAudio();
    //} else {
    //    participant->unmuteAudio();
    //}
}

bool ParticipantItemView::getAudioStatus() const
{
    //auto participant = _participant.lock();
    //if (!participant) {
    //    return false;
    //}

    //return participant->isAudioMuted();
    return false;
}

void ParticipantItemView::setVideoStatus(bool mute)
{
    //auto participant = _participant.lock();
    //if (!participant) {
    //    return;
    //}

    //if (mute) {
    //    participant->muteVideo();
    //} else {
    //    participant->unmuteVideo();
    //}
}

bool ParticipantItemView::getVideoStatus() const
{
    //auto participant = _participant.lock();
    //if (!participant) {
    //    return false;
    //}

    //return participant->isVideoMuted();
    return false;
}

void ParticipantItemView::on_toolButtonAudio_clicked(bool checked)
{
    auto vrc = _vrc.lock();
    if (!vrc) {
        DLOG("Invalid vrc");
        return;
    }

    auto participant = _participant.lock();
    if (!participant) {
        DLOG("Invalid participant");
        return;
    }

    if (checked) {
        for (const auto& stream : participant->context().streams.value_or(std::vector<vi::vr::Publisher::Stream>())) {
            if (stream.type.value_or("") == "audio") {
                vrc->mediaContrller()->muteAudio(participant->id(), stream.mid.value_or(""), false);
            }
        }
    }
    else {
        for (const auto& stream : participant->context().streams.value_or(std::vector<vi::vr::Publisher::Stream>())) {
            if (stream.type.value_or("") == "audio") {
                vrc->mediaContrller()->muteAudio(participant->id(), stream.mid.value_or(""), true);
            }
        }
    }
}

void ParticipantItemView::on_toolButtonVideo_clicked(bool checked)
{
    auto vrc = _vrc.lock();
    if (!vrc) {
        DLOG("Invalid vrc");
        return;
    }

    auto participant = _participant.lock();
    if (!participant) {
        DLOG("Invalid participant");
        return;
    }

    if (checked) {
        for (const auto& stream : participant->context().streams.value_or(std::vector<vi::vr::Publisher::Stream>())) {
            if (stream.type.value_or("") == "video") {
                vrc->mediaContrller()->muteVideo(participant->id(), stream.mid.value_or(""), false);
            }
        }
    }
    else {
        for (const auto& stream : participant->context().streams.value_or(std::vector<vi::vr::Publisher::Stream>())) {
            if (stream.type.value_or("") == "video") {
                vrc->mediaContrller()->muteVideo(participant->id(), stream.mid.value_or(""), true);
            }
        }
    }
}
