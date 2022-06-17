enum DS1307Datetime {
	SECOND,
	MINUTE
	HOUR,
	DAY_OF_WEEK,
	DAY,
	MONTH,
	YEAR,
	UNKNOWN
};

uint8_t ds1307_load(enum DS1307Datetime)
