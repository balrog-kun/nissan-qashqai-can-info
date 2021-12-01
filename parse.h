/* 2-clause BSD licensed */

/* Uitlities. TODO: move to utils.h */
static inline uint32_t get_bits(const uint8_t *bytes, uint8_t start_byte,
		uint8_t start_bit, uint8_t bits_num)
{
	uint32_t val = 0;

	while (bits_num) {
		uint8_t lsb = max((int8_t) (start_bit + 1 - bits_num), (int8_t) 0);
		uint8_t bits = start_bit + 1 - lsb;

		bits_num -= bits;
		start_bit = 7;
		val |= (uint32_t) ((bytes[start_byte++] >> lsb) & ((1 << bits) - 1)) <<
			bits_num;
	}

	return val;
}

static inline bool get_bit(const uint8_t *bytes,
		uint8_t byte_num, uint8_t bit_num)
{
	return ((bytes[byte_num] >> bit_num) & 1) == 1;
}

enum can_ign_pos {
	CAN_IGN_POS_OFF,
	CAN_IGN_POS_ACC,
	CAN_IGN_POS_ON,
	CAN_IGN_POS_START,
};

struct can_frame_35d {
	bool on;
	bool half_on;
	bool acc;
	bool panel_power;
	bool blower;
	bool rear_defrost;
	bool ac;
	enum {
		CAN_WIPERS_MODE_OFF,
		CAN_WIPERS_MODE_INTERMITTENT,
		CAN_WIPERS_MODE_SLOW,
		CAN_WIPERS_MODE_FAST,
		CAN_WIPERS_MODE_UNKNOWN,
	} wipers_mode;
	bool brake_light;
};

static inline void can_decode_35d(const uint8_t *data, struct can_frame_35d *out)
{
	out->on = ((data[0] >> 7) & 1) == 1;
	out->half_on = ((data[0] >> 6) & 1) == 1;
	out->blower = ((data[0] >> 5) & 1) == 1;
	out->rear_defrost = ((data[0] >> 1) & 1) == 1;	/* Same as bit 2? */
	out->ac = ((data[0] >> 0) & 1) == 1;
	out->panel_power = ((data[1] >> 0) & 1) == 1; /* Same as bit 0? */
	switch ((data[2] >> 5) & 1) {
	case 0:
		out->wipers_mode = can_frame_35d::CAN_WIPERS_MODE_OFF;
		break;
	case 2:
		out->wipers_mode = can_frame_35d::CAN_WIPERS_MODE_SLOW;
		break;
	case 6:
		out->wipers_mode = can_frame_35d::CAN_WIPERS_MODE_INTERMITTENT;
		break;
	case 7:
		out->wipers_mode = can_frame_35d::CAN_WIPERS_MODE_FAST;
		break;
	default:
		out->wipers_mode = can_frame_35d::CAN_WIPERS_MODE_UNKNOWN;
		break;
	}
	out->brake_light = ((data[4] >> 4) & 1) == 1;
}

const char *const can_frame_35d_wipers_mode_str[8] = {
	/* The arduino (and arduino-esp32) compiler is ancient and doesn't implement
	 * this type of initializers. */
	/*[can_frame_35d::CAN_WIPERS_MODE_OFF] = */"Off",
	/*[can_frame_35d::CAN_WIPERS_MODE_SLOW] = */"Slow",
	/*[can_frame_35d::CAN_WIPERS_MODE_INTERMITTENT] = */"Intermittent",
	/*[can_frame_35d::CAN_WIPERS_MODE_FAST] = */"Fast",
	/*[can_frame_35d::CAN_WIPERS_MODE_UNKNOWN] = */"Unknown",
};

struct can_frame_60d {
	bool driver_door_open;
	bool passenger_door_open;
	bool driver_side_rear_door_open;
	bool passenger_side_rear_door_open;
	bool trunk_door_open;
	bool position_lights_on;
	bool low_beam_lights_on;
	bool high_beam_lights_on;
	bool front_fog_lights_on;
	bool rear_fog_lights_on;
	bool hazard_lights_on;
	bool left_turn_signal;
	bool right_turn_signal;
	enum can_ign_pos ign_pos;
	bool any_door_locked;
	bool door_request_beep;
};

void can_decode_60d(const uint8_t *data, struct can_frame_60d *out)
{
	out->trunk_door_open = get_bit(data, 0, 7);
	out->passenger_side_rear_door_open = get_bit(data, 0, 6);
	out->driver_side_rear_door_open = get_bit(data, 0, 5);
	out->passenger_door_open = get_bit(data, 0, 4);
	out->driver_door_open = get_bit(data, 0, 3);
	out->position_lights_on = get_bit(data, 0, 2);
	out->low_beam_lights_on = get_bit(data, 0, 1) && !get_bit(data, 1, 3);
	out->high_beam_lights_on = get_bit(data, 0, 1) && get_bit(data, 1, 3);
	out->right_turn_signal = get_bits(data, 1, 6, 2) == 0b10;
	out->left_turn_signal = get_bits(data, 1, 6, 2) == 0b01;
	out->hazard_lights_on = get_bits(data, 1, 6, 2) == 0b11;
	switch(get_bits(data, 1, 2, 2)) {
	case 0b00:
		out->ign_pos = CAN_IGN_POS_OFF;
		break;
	case 0b01:
		out->ign_pos = CAN_IGN_POS_ACC;
		break;
	case 0b10:
		out->ign_pos = CAN_IGN_POS_START;
		break;
	case 0b11:
		out->ign_pos = CAN_IGN_POS_ON;
		break;
	}
	out->front_fog_lights_on = get_bit(data, 1, 0);
	out->rear_fog_lights_on = get_bit(data, 2, 2);
	out->any_door_locked = get_bit(data, 2, 4); /* Same as bit 3? */
	out->door_request_beep = get_bit(data, 4, 5); /* Same as bit 1? */
}
