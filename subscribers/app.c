#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rcutils/error_handling.h>
#include <std_msgs/msg/int32.h>
#include <stdio.h>
#include <unistd.h>

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

/* Number of executor handles.
 * Publishers don't count as they are driven by the timer.
 * ********** IMPORTANT: CHANGE VALUES IN app-colcon.meta.  *********
 */
#define TIMER_HANDLE_COUNT (1)
#define SUBSCRIBER_HANDLE_COUNT (1)
#define EXECUTOR_HANDLE_COUNT (TIMER_HANDLE_COUNT + SUBSCRIBER_HANDLE_COUNT)

rcl_publisher_t publisher_battery_state;
rcl_subscription_t subscriber_cmd_vel_1;
// rcl_subscription_t subscriber_cmd_vel_2;

// Logging name.
static const char *TAG = "swarm_trooper";
// Standard topic names.
static const char *k_battery_state = "battery_state";
static const char *k_cmd_vel_1 = "cmd_vel_1";
// static const char *k_cmd_vel_2 = "cmd_vel/2";
// Messages to publish.
static sensor_msgs__msg__BatteryState *battery_state_msg = NULL;

static void publish_battery_state(void) {
  battery_state_msg->voltage = 1.3;
  ESP_LOGI(TAG, "Sending msg: %f", battery_state_msg->voltage);
  rcl_ret_t rc = rcl_publish(&publisher_battery_state, battery_state_msg, NULL);
  RCLC_UNUSED(rc);
}

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  ESP_LOGI(TAG, "Timer called.");
  if (timer != NULL) {
    publish_battery_state();
  }
}

static void subscription_callback_cmd_vel_1(const void *msg_in) {
  const geometry_msgs__msg__Twist *msg =
      (const geometry_msgs__msg__Twist *)msg_in;
  ESP_LOGI(TAG, "%s called. ang.x %f", __func__, msg->angular.x);
}

// static void subscription_callback_cmd_vel_2(const void *msg_in) {
//   const geometry_msgs__msg__Twist *msg =
//       (const geometry_msgs__msg__Twist *)msg_in;
//   ESP_LOGI(TAG, "%s called. ang.x %f", __func__, msg->angular.x);
// }

void appMain(void *arg) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  // Initialise messages.
  battery_state_msg = sensor_msgs__msg__BatteryState__create();

  // Create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
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
  /* If rclc_subscription_init_default() fails, these are some of the return
    values.  Most are defined in firmware/mcu_ws/install/include/rcl/types.h
    1 = RCL_RET_ERROR = RMW_RET_ERROR - This is most common.
    10 = RCL_RET_BAD_ALLOC = RMW_RET_BAD_ALLOC
    11 = RCL_RET_INVALID_ARGUMENT
    100 = RCL_RET_ALREADY_INIT
    103 = RCL_RET_TOPIC_NAME_INVALID
    200 = RCL_RET_NODE_INVALID
    Most errors originate from rcl_subscription_init in
    firmware/mcu_ws/ros2/rcl/rcl/src/rcl/subscription.c
  */

  RCCHECK(rclc_subscription_init_default(
      &subscriber_cmd_vel_1, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), k_cmd_vel_1));

  // RCCHECK(rclc_subscription_init_default(
  //     &subscriber_cmd_vel_2, &node,
  //     ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), k_cmd_vel_2));

  // Create timer.
  ESP_LOGI(TAG, "Creating timers");
  rcl_timer_t timer = rcl_get_zero_initialized_timer();
  const unsigned int timer_timeout_ms = 1000;
  RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(timer_timeout_ms),
                                  timer_callback));

  // Create executor.
  ESP_LOGI(TAG, "Creating executor");
  rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, EXECUTOR_HANDLE_COUNT,
                             &allocator));
  unsigned int rcl_wait_timeout_ms = 1000;  // in ms
  RCCHECK(rclc_executor_set_timeout(&executor, RCL_MS_TO_NS(rcl_wait_timeout_ms)));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  ESP_LOGI(TAG, "Adding subs");
  geometry_msgs__msg__Twist twist_msg_1;
  RCCHECK(rclc_executor_add_subscription(
      &executor, &subscriber_cmd_vel_1, &twist_msg_1,
      &subscription_callback_cmd_vel_1, ON_NEW_DATA));

  // geometry_msgs__msg__Twist twist_msg_2;
  // RCCHECK(rclc_executor_add_subscription(
  //     &executor, &subscriber_cmd_vel_2, &twist_msg_2,
  //     &subscription_callback_cmd_vel_2, ON_NEW_DATA));

  // Spin forever.
  ESP_LOGI(TAG, "Spinning...");
  while (1) {
    rclc_executor_spin(&executor);
  }
  // Probably never get here but this is for completeness.
  // Free resources.
  ESP_LOGI(TAG, "Free resources");
  // RCCHECK(rcl_subscription_fini(&subscriber_cmd_vel_2, &node));
  RCCHECK(rcl_subscription_fini(&subscriber_cmd_vel_1, &node));
  RCCHECK(rcl_publisher_fini(&publisher_battery_state, &node))
  RCCHECK(rcl_node_fini(&node))
  sensor_msgs__msg__BatteryState__destroy(battery_state_msg);
  // Delete this task!
  vTaskDelete(NULL);
}
