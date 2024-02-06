#ifndef LOGGER_H // !LOGGER_H
#define LOGGER_H

#include <android/log.h>

#define TAG "AzurLane"

#define LogInfo(formatter, ...) __android_log_print(ANDROID_LOG_INFO, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogDebug(formatter, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogError(formatter, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)

#endif // !LOGGER_H