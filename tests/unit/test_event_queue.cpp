#include <gtest/gtest.h>
extern "C" {
#include "lvkw_event_queue.h"
#include "lvkw_internal.h"
}

// Mock allocator for testing
static void* mock_alloc(size_t size, void* userdata) { return malloc(size); }

static void mock_free(void* ptr, void* userdata) { free(ptr); }

class EventQueueTest : public ::testing::Test {
 protected:
  LVKW_Context_Base ctx;
  LVKW_EventQueue q;

  void SetUp() override {
    ctx.prv.alloc_cb.alloc_cb = mock_alloc;
    ctx.prv.alloc_cb.free_cb = mock_free;
    ctx.pub.userdata = nullptr;
    EXPECT_EQ(lvkw_event_queue_init(&ctx, &q, 0, 128, 2.0),
              LVKW_SUCCESS);  // 0 initial capacity to test lazy growth
  }

  void TearDown() override { lvkw_event_queue_cleanup(&ctx, &q); }
};

TEST_F(EventQueueTest, Init) {
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
  EXPECT_EQ(q.capacity, 0);
}

TEST_F(EventQueueTest, PreAllocated) {
  LVKW_EventQueue q2;
  EXPECT_EQ(lvkw_event_queue_init(&ctx, &q2, 32, 128, 2.0), LVKW_SUCCESS);
  EXPECT_EQ(q2.capacity, 32);
  EXPECT_NE(q2.pool, nullptr);
  lvkw_event_queue_cleanup(&ctx, &q2);
}

TEST_F(EventQueueTest, PushPop) {
  LVKW_Event ev = {LVKW_EVENT_TYPE_KEY};
  ev.key.key = LVKW_KEY_A;

  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);

  LVKW_Event out_ev;
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out_ev));
  EXPECT_EQ(out_ev.type, LVKW_EVENT_TYPE_KEY);
  EXPECT_EQ(out_ev.key.key, LVKW_KEY_A);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
}

TEST_F(EventQueueTest, TailCompressionScroll) {
  LVKW_Event ev1 = {LVKW_EVENT_TYPE_MOUSE_SCROLL, (LVKW_Window *)0x1};
  ev1.mouse_scroll.dx = 1.0;
  ev1.mouse_scroll.dy = 2.0;

  LVKW_Event ev2 = {LVKW_EVENT_TYPE_MOUSE_SCROLL, (LVKW_Window *)0x1};
  ev2.mouse_scroll.dx = 0.5;
  ev2.mouse_scroll.dy = -1.0;

  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev1));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev2));

  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);

  LVKW_Event out_ev;
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out_ev));
  EXPECT_DOUBLE_EQ(out_ev.mouse_scroll.dx, 1.5);
  EXPECT_DOUBLE_EQ(out_ev.mouse_scroll.dy, 1.0);
  EXPECT_EQ(out_ev.window, (LVKW_Window *)0x1);
}

TEST_F(EventQueueTest, MouseMotionMerging) {
  LVKW_Event ev1 = {LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)0x1};
  ev1.mouse_motion.dx = 10.0;
  ev1.mouse_motion.dy = 5.0;
  ev1.mouse_motion.x = 100.0;
  ev1.mouse_motion.y = 100.0;

  LVKW_Event ev2 = {LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)0x1};
  ev2.mouse_motion.dx = 2.0;
  ev2.mouse_motion.dy = -1.0;
  ev2.mouse_motion.x = 102.0;
  ev2.mouse_motion.y = 99.0;

  LVKW_Event ev3 = {LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)0x1};
  ev3.mouse_motion.dx = 1.0;
  ev3.mouse_motion.dy = 1.0;
  ev3.mouse_motion.x = -1.0;  // Simulated relative-only move
  ev3.mouse_motion.y = -1.0;

  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev1));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev2));
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev3));

  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);

  LVKW_Event out_ev;
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out_ev));
  EXPECT_DOUBLE_EQ(out_ev.mouse_motion.dx, 13.0);
  EXPECT_DOUBLE_EQ(out_ev.mouse_motion.dy, 5.0);
  EXPECT_DOUBLE_EQ(out_ev.mouse_motion.x, 102.0);  // Should be from ev2
  EXPECT_DOUBLE_EQ(out_ev.mouse_motion.y, 99.0);
  EXPECT_EQ(out_ev.window, (LVKW_Window *)0x1);
}

TEST_F(EventQueueTest, GrowthLogic) {
  // Initial capacity is 0, should grow to 64 on first push
  LVKW_Event ev = {LVKW_EVENT_TYPE_KEY};
  ev.key.key = LVKW_KEY_A;

  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_EQ(q.capacity, 64);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);

  // Fill up to 64
  for (int i = 1; i < 64; ++i) {
    ev.key.key = (LVKW_Key)(LVKW_KEY_A + i);
    lvkw_event_queue_push(&ctx, &q, &ev);
  }
  EXPECT_EQ(q.capacity, 64);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 64);

  // Push 65th event, should grow to 128 (max_capacity in SetUp)
  ev.key.key = LVKW_KEY_Z;
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  EXPECT_EQ(q.capacity, 128);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 65);
}

TEST_F(EventQueueTest, Wraparound) {
  // Force some wraparound by filling and popping
  LVKW_Event dummy = {LVKW_EVENT_TYPE_KEY};
  lvkw_event_queue_push(&ctx, &q, &dummy);  // capacity becomes 64

  // Fill almost to end
  for (int i = 0; i < 60; ++i) {
    LVKW_Event ev = {LVKW_EVENT_TYPE_KEY};
    lvkw_event_queue_push(&ctx, &q, &ev);
  }

  // Pop some to move head
  LVKW_Event out;
  for (int i = 0; i < 30; ++i) {
    lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out);
  }

  // Now head is at 30, tail is at 61.
  // Push more to wrap tail around 64
  for (int i = 0; i < 10; ++i) {
    LVKW_Event ev = {LVKW_EVENT_TYPE_KEY};
    ev.key.key = (LVKW_Key)(LVKW_KEY_0 + i);
    lvkw_event_queue_push(&ctx, &q, &ev);
  }

  // Tail should have wrapped: (61 + 10) % 64 = 7
  EXPECT_EQ(q.tail, 7);
  EXPECT_LT(q.tail, q.head);

  // Pop all and verify order/count
  uint32_t count = 0;
  while (lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out)) {
    count++;
  }
  EXPECT_EQ(count, 61 - 30 + 10);
}

TEST_F(EventQueueTest, ExhaustiveEviction) {
  // Fill the queue to max_capacity (128) with NON-compressible events
  for (int i = 0; i < 128; ++i) {
    LVKW_Event ev = {LVKW_EVENT_TYPE_KEY};
    ev.key.key = (LVKW_Key)i;
    EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &ev));
  }

  // Attempt to push another NON-compressible event (should fail as none can be
  // evicted)
  LVKW_Event critical = {LVKW_EVENT_TYPE_KEY};
  EXPECT_FALSE(lvkw_event_queue_push(&ctx, &q, &critical));

  // Attempt to push a COMPRESSIBLE event (should fail as queue is full)
  LVKW_Event motion = {LVKW_EVENT_TYPE_MOUSE_MOTION};
  EXPECT_FALSE(lvkw_event_queue_push(&ctx, &q, &motion));

  // Now pop one, push one compressible, then fill with non-compressible again
  LVKW_Event out;
  lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out);
  lvkw_event_queue_push(&ctx, &q,
                        &motion);  // Now we have 1 compressible in the queue

  // Now push a non-compressible when full (should evict the motion event)
  critical.key.key = (LVKW_Key)999;
  EXPECT_TRUE(lvkw_event_queue_push(&ctx, &q, &critical));

  bool found_critical = false;
  while (lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_ALL, &out)) {
    if (out.type == LVKW_EVENT_TYPE_KEY && out.key.key == (LVKW_Key)999) found_critical = true;
  }
  EXPECT_TRUE(found_critical);
}

TEST_F(EventQueueTest, MaskedPop) {
  LVKW_Event ev_key = {LVKW_EVENT_TYPE_KEY, (LVKW_Window *)0x1};
  LVKW_Event ev_motion = {LVKW_EVENT_TYPE_MOUSE_MOTION, (LVKW_Window *)0x2};

  lvkw_event_queue_push(&ctx, &q, &ev_key);
  lvkw_event_queue_push(&ctx, &q, &ev_motion);
  lvkw_event_queue_push(&ctx, &q, &ev_key);

  LVKW_Event out;
  // Pop only motion
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_MOUSE_MOTION, &out));
  EXPECT_EQ(out.type, LVKW_EVENT_TYPE_MOUSE_MOTION);
  EXPECT_EQ(out.window, (LVKW_Window *)0x2);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 2);

  // Pop key (should get the first one)
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_KEY, &out));
  EXPECT_EQ(out.type, LVKW_EVENT_TYPE_KEY);
  EXPECT_EQ(out.window, (LVKW_Window *)0x1);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 1);

  // Pop remaining key
  EXPECT_TRUE(lvkw_event_queue_pop(&q, LVKW_EVENT_TYPE_KEY, &out));
  EXPECT_EQ(out.type, LVKW_EVENT_TYPE_KEY);
  EXPECT_EQ(out.window, (LVKW_Window *)0x1);
  EXPECT_EQ(lvkw_event_queue_get_count(&q), 0);
}
