enum DS1307Datetime {
	SECOND,
	MINUTE,
	HOUR,
	DATE,
	DAY,
	MONTH,
	YEAR
};

uint8_t ds1307_load(enum DS1307Datetime);
void    ds1307_save(enum DS1307Datetime type, uint8_t * value);
