#include "realtime_component.h"
#include "hiredis/hiredis.h"
#include "hiredis/hiredis_ssl.h"

#include <iostream>
#include <filesystem>
#include <thread>
#include <string>

#include "json_animation.h"
#include "log.h"
#include "pose_component.h"
#include "transform_component.h"
#include "json/reader.h"
#include "json/value.h"
#include "entity/entity.h"
#include "components/pose_component.h"

anim::RealTimeComponent::RealTimeComponent()
	: ip_("127.0.0.1"), port_(6379), conn_(nullptr), resp_(nullptr), b_is_connected_(false), db_key_("animation")
{
}

anim::RealTimeComponent::~RealTimeComponent()
{
	disconnect();
}

void anim::RealTimeComponent::pre_update()
{
	if (!b_is_connected_)
	{
		Bone::SetUseInterpolate(true);
		return;
	}
	Bone::SetUseInterpolate(false);

	try
	{
		auto reply_guard = RedisGuard((redisReply*) redisCommand(conn_, "GET %s", "animation"));	// db_key_.data());
		Json::Value bones;
		Json::Reader reader;
		if (reply_guard.get()->type == REDIS_REPLY_NIL)
		{
			return;
		}
		bool parsingSuccessful = reader.parse(std::string(reply_guard.get()->str), bones);
		if (!parsingSuccessful)
		{
			anim::LOG("Failed to parse the data");
			return;
		}

		uint32_t size = static_cast<uint32_t>(bones.size());

		for (uint32_t idx = 0; idx < size; idx++)
		{
			std::string bone_name = bones[idx].get("name", "").asString();

			auto position = JsonUtil::GetPosition(bones[idx]["position"]);
			auto rotation = JsonUtil::GetRotation(bones[idx]["rotation"]);
			auto scale = JsonUtil::GetScale(bones[idx]["scale"]);
			glm::mat4 transform =
				glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
			get_owner()->get_component<PoseComponent>()->insert_and_update_bone(bone_name, transform);
		}
	}
	catch (std::exception& e)
	{
#ifndef NDEBUG
		std::cout << e.what() << std::endl;
#endif
	}
}

void anim::RealTimeComponent::connect()
{
	try
	{
		if (b_is_connected_)
			return;
		conn_ = redisConnect(ip_.data(), port_);

		if ((nullptr != conn_) && conn_->err)
		{
			anim::LOG("Can't connect to Redis server with Error: ");
			anim::LOG(conn_->errstr);
			anim::LOG("==========================================");
			redisFree(conn_);
			conn_ = nullptr;
		}
		else if (nullptr == conn_)
		{
			anim::LOG("Can't connect to Redis server");
			return;
		}
		anim::LOG("redis connected");
		b_is_connected_ = true;
	}
	catch (std::exception& e)
	{
		b_is_connected_ = false;
	}
}

void anim::RealTimeComponent::disconnect()
{
	if (b_is_connected_)
	{
		resp_ = (redisReply*) redisCommand(conn_, "LPUSH mq {\"end\":\"1\"}");
		freeReplyObject(resp_);
		redisFree(conn_);
		resp_ = nullptr;
		conn_ = nullptr;
		b_is_connected_ = false;
	}
}
