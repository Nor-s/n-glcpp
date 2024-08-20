#ifndef ANIM_ENTITY_COMPONENT_REALTIME_COMPONENT_H
#define ANIM_ENTITY_COMPONENT_REALTIME_COMPONENT_H

#include "component.h"
#include <string>

#include "hiredis.h"

struct redisContext;
struct redisReply;

namespace anim
{
	class RedisGuard
	{
	public:
		RedisGuard(redisReply* reply)
			: reply_(reply)
		{
		}
		~RedisGuard()
		{
			if (reply_)
			{
				freeReplyObject(reply_);
			}
		}
		redisReply* get()
		{
			return reply_;
		}
	private:
		redisReply* reply_;
	};
	class RealTimeComponent : public ComponentBase<RealTimeComponent>
	{
	public:
		RealTimeComponent();
		~RealTimeComponent();

		virtual void pre_update() override;

		void connect();
		void disconnect();
		bool is_connected() const { return b_is_connected_; }

	private:
		std::string ip_;
		uint32_t port_;
		redisContext* conn_ = nullptr;
		redisReply* resp_ = nullptr;
		bool b_is_connected_ = false;
		std::string db_key_;
	};
}
#endif