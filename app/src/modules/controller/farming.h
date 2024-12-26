#ifndef FARMING_H
#define FARMING_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/data/json.h>

/**
 * @brief Farming config message structure
 * "farmings": [
	{
	    "start": "2024-10-24T12:00:00Z",
	    "end": "2024-10-24T14:00:00Z",
	    "triggerType": 0,   // Can be 0 (Time) or 1 (Sensor)
	    "actionType": 0,    // Can be 0 (Off) or 1 (On)
	    "actuatorId": 0,    // Can be 0 (Pump) or 1 (LED)
	    "activationTime": 2,     // Activation time (hour 0-23)
	    "amount": 60        //  amount of seconds after which we need to invert the action
	},
	{
	    "start": "2024-10-25T08:00:00Z",
	    "end": "2024-10-25T10:00:00Z",
	    "triggerType": 1,   // Can be 0 (Time) or 1 (Sensor)
	    "actionType": 1,    // Can be 0 (Off) or 1 (On)
	    "actuatorId": 1,    // Can be 0 (Pump) or 1 (LED)
	    "sensorId": 0,      // Can be 0 (Soil) or 1 (Air)
	    "target": 70,       // Do the action until the sensor value is equal to the target value
	    "comparison": 2     // Can be 0 (<), 1 (>), or 2 (=)
	}
    ]
 *
 */

struct farming_action {
	int start; // ms since epoch
	int end;   // ms since epoch
	int triggerType;
	int actionType;
	int actuatorId;
	int activationTime;
	int amount;
	int sensorId;
	int target;
	int comparison;
	bool executed;
};

struct farming {
	struct farming_action actions[10];
	size_t num_elements;
};

/// @brief Farming config message structure
struct config_farming_message {
	struct farming_action actions[10];
	size_t num_elements;
};

static const struct json_obj_descr time_action_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct farming_action, start, JSON_TOK_UINT64),
	JSON_OBJ_DESCR_PRIM(struct farming_action, end, JSON_TOK_UINT64),
	JSON_OBJ_DESCR_PRIM(struct farming_action, triggerType, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, actionType, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, actuatorId, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, activationTime, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, amount, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, sensorId, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, target, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, comparison, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct farming_action, executed, JSON_TOK_FALSE),
};

static const struct json_obj_descr farming_array_descr[] = {
	JSON_OBJ_DESCR_OBJ_ARRAY(struct config_farming_message, actions, 10, num_elements,
				 time_action_descr, ARRAY_SIZE(time_action_descr)),
};

static struct farming farming_results;

// void testParseArray()
// {
// 	int ret;

// 	/* decode a single object */
// 	uint8_t json_msg[] =
// 		"{\"actions\":[{\"start\":1732356017,\"end\":1763892017,\"triggerType\":0,"
// 		"\"actionType\":0,\"actuatorId\":0,\"interval\":30,\"amount\":60,"
// 		"\"sensorId\":1,\"target\":0,\"comparison\":0}],\"num_elements\":1}";

// 	/**
// 	 * Returns < 0 if error, bitmap of decoded fields on success (bit 0 is set if first field in
// 	 * the descriptor has been properly decoded, etc).
// 	 */
// 	ret = json_obj_parse(json_msg, sizeof(json_msg), farming_array_descr,
// 			     ARRAY_SIZE(farming_array_descr), &farming_results);

// 	// TODO: why this doesn't work!!!!!

// 	if (ret < 0) {
// 		LOG_ERR("JSON Parse Error: %d", ret);
// 	} else {
// 		LOG_INF("json_obj_parse return code: %d", ret);

// 		// log every element
// 		for (int i = 0; i < farming_results.num_elements; i++) {
// 			LOG_INF("start: %d", farming_results.actions[i].start);
// 			LOG_INF("end: %d", farming_results.actions[i].end);
// 		}
// 	}
// }

#endif // FARMING_H