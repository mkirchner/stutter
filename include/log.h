/*
 * log.h
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOGLEVEL LOGLEVEL_DEBUG

enum {
	LOGLEVEL_CRITICAL, // 0
	LOGLEVEL_WARNING, // 1
	LOGLEVEL_INFO, // 2
	LOGLEVEL_DEBUG, // 3
	LOGLEVEL_NONE // 4
};

extern const char* log_level_strings[];

#define log(level, fmt, ...) \
    do { if (level <= LOGLEVEL) fprintf(stderr, "[%s] %s:%s:%d: " fmt "\n", log_level_strings[level], __func__, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)

#define LOG_CRITICAL(fmt, ...) log(LOGLEVEL_CRITICAL, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log(LOGLEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log(LOGLEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log(LOGLEVEL_DEBUG, fmt, ##__VA_ARGS__)

#endif /* !__LOG_H__ */
