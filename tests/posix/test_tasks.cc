#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/osal.h"
#include "test_utils.h"

namespace test_tasks {

using testutils::wait_nanoseconds;

/* the following test runs N threads, each of which hold a common
   mutex, and each of which use a common counter.

   Each thread loops, and in each iteration, will pause a random
   time (in order to increase chance of concurrent access), get the
   mutex, pause a bit more, increase the coutner, and return the
   mutex.

   The rationale of the test is that if the mutex would not
   protect shared data properly, we would have some missing
   counts due to race conditions.
*/
typedef struct {
  int thread_id;
  uint loopcount;
  uint max_wait_time_nsec;
  unsigned long *p_counter;
  pthread_mutex_t *p_count_mutex;
} thread_param_t;

void *test_random(void *p_params) {
  thread_param_t params = *((thread_param_t *)p_params);

  const int thread_id = params.thread_id;
  const uint max_wait_time = params.max_wait_time_nsec;
  // osal_retval_t orv;
  int rv;

  if (max_wait_time > 0) {
    srand(thread_id);
  }
  for (uint i = 0; i < params.loopcount; i++) {
    // randomly wait
    if ((max_wait_time > 0) && (rand() % 2)) {
      wait_nanoseconds(rand() % max_wait_time);
    }

    rv = pthread_mutex_lock(params.p_count_mutex);
    EXPECT_EQ(rv, OSAL_OK) << "pthread_mutex_lock() failed";

    unsigned long old_value = *(params.p_counter);
    // randomly wait
    if ((max_wait_time > 0) && (rand() % 2)) {
      wait_nanoseconds(rand() % max_wait_time);
    }

    // increment shared counter
    *(params.p_counter) = old_value + 1;

    // return lock
    rv = pthread_mutex_unlock(params.p_count_mutex);
    EXPECT_EQ(rv, OSAL_OK) << "pthread_mutex_unlock() failed";
  }

  return nullptr;
}

TEST(TasksMultithreading, Parallel) {
  const ulong N_THREADS = 100;
  const uint LOOPCOUNT = 100000;

  osal_task_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  pthread_mutex_t count_mutex;
  unsigned long counter = 0;
  osal_retval_t orv;
  int rv;

  bool verbose = (getenv("VERBOSE") != nullptr);

  rv = pthread_mutex_init(&count_mutex, nullptr);
  ASSERT_EQ(rv, 0) << "pthread_mutex_init() failed";

  for (ulong i = 0; i < N_THREADS; i++) {
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = 0;

    if (verbose) {
      printf("starting thread %lu\n", i);
    }
    orv = osal_task_create(/*thread*/ &(thread_ids[i]),
                           /*osal_task_attr*/ nullptr,
                           /* start_routine */ test_random,
                           /* arg */ (void *)&(thread_params[i]));
    ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() failed";
  }
  for (ulong i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    orv = osal_task_join(/*thread*/ &thread_ids[i],
                         /*retval*/ nullptr);
    ASSERT_EQ(orv, OSAL_OK) << "osal_task_join() failed";
  }
  rv = pthread_mutex_destroy(&count_mutex);
  ASSERT_EQ(orv, 0) << "pthread_mutex_init() failed";

  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
           N_THREADS * LOOPCOUNT, counter);
  }

  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
      << "multi-threaded counter test failed";
}

TEST(TasksMultithreading, RandomizedPlusWait) {
  const ulong N_THREADS = 8;
  const uint LOOPCOUNT = 10000;
  const uint MAX_WAIT_TIME_NSEC = 500;

  osal_task_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  pthread_mutex_t count_mutex;
  unsigned long counter = 0;
  int rv;
  osal_retval_t orv;

  bool verbose = (getenv("VERBOSE") != nullptr);

  rv = pthread_mutex_init(&count_mutex, nullptr);
  ASSERT_EQ(rv, 0) << "pthread_mutex_init() failed";

  for (ulong i = 0; i < N_THREADS; i++) {
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = MAX_WAIT_TIME_NSEC;

    if (verbose) {
      printf("starting thread %lu\n", i);
    }
    orv = osal_task_create(/*thread*/ &(thread_ids[i]),
                           /*osal_task_attr*/ nullptr,
                           /* start_routine */ test_random,
                           /* arg */ (void *)&(thread_params[i]));
    ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() failed";
  }
  // should complete in about 50 ms
  for (ulong i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    orv = osal_task_join(/*thread*/ &thread_ids[i],
                         /*retval*/ nullptr);
    ASSERT_EQ(orv, OSAL_OK) << "osal_task_join() failed";
  }
  rv = pthread_mutex_destroy(&count_mutex);
  ASSERT_EQ(rv, 0) << "pthread_mutex_destroy() failed";

  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
           N_THREADS * LOOPCOUNT, counter);
  }

  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
      << "multi-threaded counter test failed";
}

} // namespace test_tasks

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
