#pragma once

#include <queue>

namespace wss {
enum class AdvertBehaviorTest {
	NONE, MOVE_TO, WAIT, AWARD_ATTRIBUTE
};

/// Temporary test implementation for Advertisement Command: a class for deferred command with associated behavior tree.
/// Note this is just test. Future version will contain real behavior tree. Each behavior can have a duck-typed data array associated.
/// Each token behavior can collate data from other components and can also include data from attached duck-typed data array.
class AdvertCommand {
public:
	AdvertCommand() : _name("default") {}

	AdvertCommand(const std::string &name, std::queue<AdvertBehaviorTest> behaviors, std::queue<glm::vec2> behaviorData) :
			_name(name), _behaviors(behaviors), _behaviorData(behaviorData) {

	}
	virtual ~AdvertCommand() {
	}

	int getBehaviorTreeSize() {
		return _behaviors.size();
	}

	///Gets behavior tree associated with this command.
	AdvertBehaviorTest getBehaviorTree() {

		AdvertBehaviorTest behavior = AdvertBehaviorTest::NONE;

		if (_behaviors.size() > 0) {
			behavior = _behaviors.front();
			_behaviors.pop();
		}

		return behavior;
	}

	glm::vec2 popData() {
		glm::vec2 data;
		if (_behaviorData.size() > 0) {
			data = _behaviorData.front();
			_behaviorData.pop();
		}

		return data;
	}

private:
	std::string _name;

	std::queue<AdvertBehaviorTest> _behaviors;
	std::queue<glm::vec2> _behaviorData;

};
}
