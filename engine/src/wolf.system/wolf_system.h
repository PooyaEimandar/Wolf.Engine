#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// w_timespec_t for storing time point
struct w_timespec_t {
  /// total seconds
  uint64_t secs;
  /// the fractional part in nanoseconds.
  uint32_t nano_seconds;
  /// true means overflowed
  bool overflowed;
};

/// w_chrono_t
struct w_chrono_t {
  void *ptr;
};

/// unsigned 128 bit integer
struct w_u128 {
  /// the upper bytes
  uint64_t upper;
  /// the lower bytes
  uint64_t lower;
};

extern "C" {

/// Get C timespec
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
w_timespec_t *w_chrono_clock_now(uint32_t p_clock_type);

/// Get C timespec in total seconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
double w_chrono_clock_now_secs(uint32_t p_clock_type);

/// Get C timespec in total milliseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
double w_chrono_clock_now_millis(uint32_t p_clock_type);

/// Get C timespec in total microseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
double w_chrono_clock_now_micros(uint32_t p_clock_type);

/// Get C timespec in total nanoseconds
/// # Arguments
///
/// * `p_clock_type` - pointer to chrono
///     CLOCK_MONOTONIC : represents the absolute elapsed wall-clock time since some arbitrary, fixed point in the past. It isn't affected by changes in the system time-of-day clock or NTP.
double w_chrono_clock_now_nanos(uint32_t p_clock_type);

/// Initialize and returns instant chrono object
w_chrono_t *w_chrono_init_now();

/// Release instant chrono object
/// # Arguments
///
/// * `p` - pointer to chrono object
void w_chrono_fini(w_chrono_t **ptr);

/// Returns elapsed timespec since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
w_timespec_t *w_chrono_elapsed(w_chrono_t *p);

/// Returns whole elapsed time in milliseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
w_u128 w_chrono_elapsed_millis(w_chrono_t *p);

/// Returns whole elapsed time in microseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
w_u128 w_chrono_elapsed_micros(w_chrono_t *p);

/// Returns whole elapsed time in nanoseconds since function w_chrono_init called
/// # Arguments
///
/// * `p` - pointer to chrono object
w_u128 w_chrono_elapsed_nanos(w_chrono_t *p);

/// Returns duration between two chrono times
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
w_timespec_t *w_chrono_duration(w_chrono_t *p1, w_chrono_t *p2);

/// Returns duration between two chrono times in total milliseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
w_u128 w_chrono_duration_millis(w_chrono_t *p1, w_chrono_t *p2);

/// Returns duration between two chrono times in total microseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
w_u128 w_chrono_duration_micros(w_chrono_t *p1, w_chrono_t *p2);

/// Returns duration between two chrono times in total nanoseconds
/// # Arguments
///
/// * `p1` - pointer to chrono object
/// * `p2` - pointer to chrono object
w_u128 w_chrono_duration_nanos(w_chrono_t *p1, w_chrono_t *p2);

/// Convert total days to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
w_timespec_t *w_timespec_from_zero();

/// Convert total days to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
w_timespec_t *w_timespec_from_days(uint64_t p_val);

/// Convert total hours to w_timespec_t
/// # Arguments
///
/// * `p_val` - hours
w_timespec_t *w_timespec_from_hours(uint64_t p_val);

/// Convert total minutes to w_timespec_t
/// # Arguments
///
/// * `p_val` - minutes
w_timespec_t *w_timespec_from_mins(uint64_t p_val);

/// Convert total seconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - seconds
w_timespec_t *w_timespec_from_secs(uint64_t p_val);

/// Convert total milliseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - milliseconds
w_timespec_t *w_timespec_from_millis(uint64_t p_val);

/// Convert total microseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - microseconds
w_timespec_t *w_timespec_from_micros(uint64_t p_val);

/// Convert total nanoseconds to w_timespec_t
/// # Arguments
///
/// * `p_val` - nanoseconds
w_timespec_t *w_timespec_from_nanos(uint64_t p_val);

/// Convert long time to w_timespec_t
/// # Arguments
///
/// * `p_days` - days
/// * `p_hours` - hours
/// * `p_mins` - mins
/// * `p_secs` - seconds
/// * `p_millis` - milliseconds
/// * `p_micros` - microseconds
/// * `p_nanos` - nanoseconds
w_timespec_t *w_timespec_from_long_time(uint64_t p_days,
                                        uint64_t p_hours,
                                        uint64_t p_mins,
                                        uint64_t p_secs,
                                        uint64_t p_millis,
                                        uint64_t p_micros,
                                        uint64_t p_nanos);

/// pL += pR
/// # Arguments
///
/// * `p_lval` - left value
/// * `p_rval` - right value
bool w_timespec_add(w_timespec_t **p_lval, w_timespec_t *p_rval);

} // extern "C"
