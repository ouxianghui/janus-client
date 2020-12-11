#include "participant_item_view.h"
#include "participant.h"

ParticipantItemView::ParticipantItemView(std::shared_ptr<vi::Participant> participant, QWidget *parent)
	: QWidget(parent)
    , _participant(participant)
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

}

void ParticipantItemView::on_toolButtonVideo_clicked(bool checked)
{

}
