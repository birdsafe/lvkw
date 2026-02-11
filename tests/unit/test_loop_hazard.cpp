#include <gtest/gtest.h>
extern "C" {
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"
}

// Mock allocator for testing
static void* mock_alloc(size_t size, void* userdata) { return malloc(size); }
static void mock_free(void* ptr, void* userdata) { free(ptr); }

class EventQueueHazardTest : public ::testing::Test {
 protected:
  LVKW_Context_Base ctx;
  LVKW_EventQueue q;

  void SetUp() override {
    ctx.prv.alloc_cb.alloc_cb = mock_alloc;
    ctx.prv.alloc_cb.free_cb = mock_free;
    ctx.pub.userdata = nullptr;
    // Small capacity to see effects quickly
    EXPECT_EQ(lvkw_event_queue_init(&ctx, &q, 4, 4, 2.0), LVKW_SUCCESS);
  }

  void TearDown() override { lvkw_event_queue_cleanup(&ctx, &q); }
};

TEST_F(EventQueueHazardTest, RemoveWindowEventsDecrementsCount) {
  LVKW_Window* win1 = (LVKW_Window*)0x1;
  LVKW_Event ev = {LVKW_EVENT_TYPE_KEY, win1};

  lvkw_event_queue_push(&ctx, &q, &ev);
  lvkw_event_queue_push(&ctx, &q, &ev);
  
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 2);

  // Remove events for win1
  lvkw_event_queue_remove_window_events(&q, win1);

  // FIXED: Count should be 0.
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);

  // Now pop should return false
  LVKW_Event out;
  EXPECT_FALSE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out));
}

TEST_F(EventQueueHazardTest, PopCorrectlySkipsTombstones) {
  LVKW_Window* win1 = (LVKW_Window*)0x1;
  LVKW_Event ev = {LVKW_EVENT_TYPE_KEY, win1};

  // Fill the queue
  lvkw_event_queue_push(&ctx, &q, &ev);
  lvkw_event_queue_push(&ctx, &q, &ev);
  lvkw_event_queue_push(&ctx, &q, &ev);
  lvkw_event_queue_push(&ctx, &q, &ev);

  lvkw_event_queue_remove_window_events(&q, win1);

  EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
  
  LVKW_Event out;
  EXPECT_FALSE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out));
}

TEST_F(EventQueueHazardTest, PushFailsDueToIncorrectCount) {
  LVKW_Window* win1 = (LVKW_Window*)0x1;
  LVKW_Event ev = {LVKW_EVENT_TYPE_KEY, win1};

  // Fill the queue to its max capacity (4)
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 4);

  // Mark all as tombstoned
  lvkw_event_queue_remove_window_events(&q, win1);

  // The queue is effectively EMPTY now, and count should be 0.
  // Pushing a new event should now SUCCEED.
  LVKW_Event ev2 = {LVKW_EVENT_TYPE_KEY, (LVKW_Window*)0x2};
  
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev2));
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);
}
