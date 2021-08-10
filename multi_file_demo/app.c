#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rcutils/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <stdio.h>
#include <unistd.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#include "drivers.h"
#include "esp_log.h"
#include "geometry_msgs/msg/twist.h"
#include "sensor_msgs/msg/battery_state.h"

#define RCCHECK(fn)                                                 \
  {                                                                 \
    rcl_ret_t temp_rc = fn;                                         \
    if ((temp_rc != RCL_RET_OK)) {                                  \
      printf("Failed status on line %d: %d. Aborting.\n", __LINE__, \
             (int)temp_rc);                                         \
      vTaskDelete(NULL);                                            \
    }                                                               \
  }
#define RCSOFTCHECK(fn)                                               \
  {                                                                   \
    rcl_ret_t temp_rc = fn;                                           \
    if ((temp_rc != RCL_RET_OK)) {                                    \
      printf("Failed status on line %d: %d. Continuing.\n", __LINE__, \
             (int)temp_rc);                                           \
    }                                                                 \
  }

// Tick definitions.
#define TICK_RATE_HZ (10)
#define MS_PER_TICK (1000 / TICK_RATE_HZ)
#define US_PER_TICK (MS_PER_TICK * 1000)

// Number of executor handles: 1 timer, 1 subscribers, 0 services.
// Publishers don't count as they are driven by the timer.
// NOTE: UPDATE app-colcon.meta IF YOU CHANGE THIS VALUE!
#define EXECUTOR_HANDLE_COUNT (2)

rcl_publisher_t publisher_battery_state;
rcl_subscription_t subscriber_cmd_vel;

// Logging name.
static const char *TAG = "rover";
// Standard topic/service names.
static const char *k_battery_state = "battery_state";
static const char *k_cmd_vel = "cmd_vel";
// Messages to publish.
static sensor_msgs__msg__BatteryState *battery_state_msg = NULL;

static void publish_battery_state(void) {
  int milli_volts = 0;
  int milli_amps = 0;
  driver_get_battery_state(&milli_volts, &milli_amps);
  // Convert from milli-Volts to Volts.
  msg->voltage = (float)(milli_volts / 1000);
  // Convert from milli-Amps to Amps.
  msg->current (float)(milli_amps / 1000);
  // Adjust these to suit you application.
  msg->power_supply_technology =
      sensor_msgs__msg__BatteryState__POWER_SUPPLY_TECHNOLOGY_LIPO;
  msg->present = true;
  ESP_LOGI(TAG, "Sending battery state: %f", battery_state_msg->voltage);
  rcl_ret_t rc = rcl_publish(&publisher_battery_state, battery_state_msg, NULL);
  RCLC_UNUSED(rc);
}

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  ESP_LOGI(TAG, "Timer called.");
  if (timer != NULL) {
    publish_battery_state();
  }
}

static void subscriber_cmd_vel(const void *msg_in) {
  const geometry_msgs__msg__Twist *msg =
    (const geometry_msgs__msg__Twist *)msg_in;
  int forward_mm_per_second = msg->linear.x * 1000;
  int rotation_milli_radians = msg->angular.z * 1000;
  drivers_set_cmd_vel(forward_mm_per_second, rotation_milli_radians);
}

void appMain(void *arg) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  // Create messages.
  battery_state_msg = sensor_msgs__msg__BatteryState__create();

  // Create init_options.
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node.
  rcl_node_t node = rcl_get_zero_initialized_node();
  RCCHECK(rclc_node_init_default(&node, TAG, "", &support));

  // Create publishers.
  ESP_LOGI(TAG, "Creating publishers");

  RCCHECK(rclc_publisher_init_default(
      &publisher_battery_state, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, BatteryState),
      k_battery_state));

  // Create subscribers.
  ESP_LOGI(TAG, "Creating subscribers");
  RCCHECK(rclc_subscription_init_default(
      &subscriber_cmd_vel, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), k_cmd_vel));

  // Create timer.
  ESP_LOGI(TAG, "Creating timers");
  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(timer_timeout),
                                  timer_callback));

  // Create executor.
  ESP_LOGI(TAG, "Creating executor");
  rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, EXECUTOR_HANDLE_COUNT,
                             &allocator));
  unsigned int rcl_wait_timeout = 1000;  // in ms
  RCCHECK(rclc_executor_set_timeout(&executor, RCL_MS_TO_NS(rcl_wait_timeout)));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // Add cmd_vel subscriber.
  ESP_LOGI(TAG, "Adding cmd_vel sub");
  geometry_msgs__msg__Twist twist_msg;
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber_cmd_vel,
                                         &twist_msg, &messages_cmd_vel,
                                         ON_NEW_DATA));

  // On an ESP32 until the power is disconnected or reset pressed.
  ESP_LOGI(TAG, "Spinning...");
  while (1) {
    rclc_executor_spin_some(&executor, 100);
    usleep(US_PER_TICK);
  }

  // Free resources.
  ESP_LOGI(TAG, "Free resources");
  RCCHECK(rcl_subscription_fini(&subscriber_cmd_vel, &node));
  RCCHECK(rcl_publisher_fini(&publisher_battery_state, &node))
  RCCHECK(rcl_node_fini(&node))
  sensor_msgs__msg__BatteryState__destroy(battery_state_msg);

  vTaskDelete(NULL);
}
