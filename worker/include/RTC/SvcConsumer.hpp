#ifndef MS_RTC_SVC_CONSUMER_HPP
#define MS_RTC_SVC_CONSUMER_HPP

#include "RTC/Codecs/PayloadDescriptorHandler.hpp"
#include "RTC/Consumer.hpp"
#include "RTC/RtpStreamSend.hpp"
#include "RTC/SeqManager.hpp"
#include <map>

namespace RTC
{
	class SvcConsumer : public RTC::Consumer, public RTC::RtpStreamSend::Listener
	{
	public:
		SvcConsumer(
		  const std::string& id,
		  const std::string& producerId,
		  RTC::Consumer::Listener* listener,
		  json& data);
		~SvcConsumer() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) const override;
		void FillJsonScore(json& jsonObject) const override;
		void HandleRequest(Channel::ChannelRequest* request) override;
		RTC::Consumer::Layers GetPreferredLayers() const override
		{
			RTC::Consumer::Layers layers;

			layers.spatial  = this->preferredSpatialLayer;
			layers.temporal = this->preferredTemporalLayer;

			return layers;
		}
		bool IsActive() const override
		{
			// clang-format off
			return (
				RTC::Consumer::IsActive() &&
				this->producerRtpStream &&
				(this->producerRtpStream->GetScore() > 0u || this->producerRtpStream->HasDtx())
			);
			// clang-format on
		}
		void ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void ProducerRtcpSenderReport(RTC::RtpStream* rtpStream, bool first) override;
		uint8_t GetBitratePriority() const override;
		uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
		void ApplyLayers() override;
		uint32_t GetDesiredBitrate() const override;
		void SendRtpPacket(std::shared_ptr<RTC::RtpPacket> packet) override;
		void GetRtcp(RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs) override;
		std::vector<RTC::RtpStreamSend*> GetRtpStreams() override
		{
			return this->rtpStreams;
		}
		void NeedWorstRemoteFractionLost(uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;
		void ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket) override;
		void ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc) override;
		void ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report) override;
		void ReceiveRtcpXrReceiverReferenceTime(RTC::RTCP::ReceiverReferenceTime* report) override;
		uint32_t GetTransmissionRate(uint64_t nowMs) override;
		float GetRtt() const override;

	private:
		void UserOnTransportConnected() override;
		void UserOnTransportDisconnected() override;
		void UserOnPaused() override;
		void UserOnResumed() override;
		void CreateRtpStream();
		void RequestKeyFrame();
		void MayChangeLayers(bool force = false);
		bool RecalculateTargetLayers(int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const;
		void UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer);
		void EmitScore() const;
		void EmitLayersChange() const;

		/* Pure virtual methods inherited from RtpStreamSend::Listener. */
	public:
		void OnRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamRetransmitRtpPacket(RTC::RtpStreamSend* rtpStream, RTC::RtpPacket* packet) override;

	private:
		// Allocated by this.
		RTC::RtpStreamSend* rtpStream{ nullptr };
		// Others.
		std::vector<RTC::RtpStreamSend*> rtpStreams;
		RTC::RtpStream* producerRtpStream{ nullptr };
		bool syncRequired{ false };
		RTC::SeqManager<uint16_t> rtpSeqManager;
		int16_t preferredSpatialLayer{ -1 };
		int16_t preferredTemporalLayer{ -1 };
		int16_t provisionalTargetSpatialLayer{ -1 };
		int16_t provisionalTargetTemporalLayer{ -1 };
		std::unique_ptr<RTC::Codecs::EncodingContext> encodingContext;
		uint64_t lastBweDowngradeAtMs{ 0u }; // Last time we moved to lower spatial layer due to BWE.
	};
} // namespace RTC

#endif
