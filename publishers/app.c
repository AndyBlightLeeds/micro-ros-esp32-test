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

#include "esp_log.h"
#include "sensor_msgs/msg/range.h"

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

// Number of executor handles: 1 timer, 0 subscribers, 0 services.
// Publishers don't count as they are driven by the timer.
// NOTE: UPDATE app-colcon.meta IF YOU CHANGE THIS VALUE!
#define EXECUTOR_HANDLE_COUNT (1)

rcl_publisher_t publisher_range_1;
rcl_publisher_t publisher_range_2;
rcl_publisher_t publisher_range_3;
rcl_publisher_t publisher_range_4;
rcl_publisher_t publisher_range_5;
rcl_publisher_t publisher_range_6;

// Logging name.
static const char *TAG = "test";
// Standard topic/service names.
static const char *k_range_1 = "sensors/tof1";
static const char *k_range_2 = "sensors/tof2";
static const char *k_range_3 = "sensors/tof3";
static const char *k_range_4 = "sensors/tof4";
static const char *k_range_5 = "sensors/tof5";
static const char *k_range_6 = "sensors/tof6";

// Messages to publish.  Be lazy and use the same message for all range sensors.
static sensor_msgs__msg__Range *range_msg = NULL;

static void publish_range_1(void) {
  // ToF so say infrared.
  range_msg->radiation_type = sensor_msgs__msg__Range__INFRARED;
  range_msg->field_of_view = 0.1;
  range_msg->min_range = 0.1;
  range_msg->max_range = 4.0;
  range_msg->range = 1.1;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_1, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void publish_range_2(void) {
  range_msg->range = 1.2;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_2, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void publish_range_3(void) {
  range_msg->range = 1.3;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_3, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void publish_range_4(void) {
  range_msg->range = 1.4;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_4, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void publish_range_5(void) {
  range_msg->range = 1.5;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_5, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void publish_range_6(void) {
  range_msg->range = 1.6;
  ESP_LOGI(TAG, "Sending range: %f", range_msg->range);
  rcl_ret_t rc = rcl_publish(&publisher_range_6, range_msg, NULL);
  RCLC_UNUSED(rc);
}

static void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
  ESP_LOGI(TAG, "Timer called.");
  if (timer != NULL) {
    publish_range_1();
    publish_range_2();
    publish_range_3();
    publish_range_4();
    publish_range_5();
    publish_range_6();
  }
}

void appMain(void *arg) {
  rcl_allocator_t allocator = rcl_get_default_allocator();
  rclc_support_t support;

  // Create messages.
  range_msg = sensor_msgs__msg__Range__create();

  // Create init_options.
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node.
  rcl_node_t node = rcl_get_zero_initialized_node();
  RCCHECK(rclc_node_init_default(&node, TAG, "", &support));

  // Create publishers.
  ESP_LOGI(TAG, "Creating publishers");

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_1, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_1));

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_2, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_2));

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_3, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_3));

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_4, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_4));

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_5, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_5));

  RCCHECK(rclc_publisher_init_default(
      &publisher_range_6, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
      k_range_6));

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


  // Spin until the power is disconnected or reset pressed.
  ESP_LOGI(TAG, "Spinning...");
  while (1) {
    rclc_executor_spin_some(&executor, 100);
    usleep(US_PER_TICK);
  }

  // Free resources.  Probably never called on the ESP32.
  ESP_LOGI(TAG, "Free resources");
  RCCHECK(rcl_publisher_fini(&publisher_range_1, &node))
  RCCHECK(rcl_publisher_fini(&publisher_range_2, &node))
  RCCHECK(rcl_publisher_fini(&publisher_range_3, &node))
  RCCHECK(rcl_publisher_fini(&publisher_range_4, &node))
  RCCHECK(rcl_publisher_fini(&publisher_range_5, &node))
  RCCHECK(rcl_node_fini(&node))
  sensor_msgs__msg__Range__destroy(range_msg);

  vTaskDelete(NULL);
}
